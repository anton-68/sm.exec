/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
FSM class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "sm_fsm.h"
#include "../lib/jsmn/jsmn.h"

char *sm_node_type_to_string[4] = {"Process", "State", "Initial", "Final"};
char *sm_transition_type_to_string[5] = {"Regular", "Cascade", "Hierarchical", "Asynchronous", "Static(FSM)"};

static inline sm_fsm *cleanup(sm_fsm *fsm, jsmntok_t *tokens)
    __attribute__((always_inline));
static inline char *tostr(const char *f, jsmntok_t *t)
    __attribute__((always_inline));
static inline bool streq(const char *js, jsmntok_t *t, const char *s)
    __attribute__((always_inline));
static inline int toint(const char *f, jsmntok_t *t)
    __attribute__((always_inline));

void sm_fsm_destroy(sm_fsm **sm)
{
    sm_fsm *f = *sm;
    if (f->nodes != NULL)
    {
        for (size_t i = 0; i < f->num_of_nodes; i++)
        {
            if (f->nodes[i].transitions != NULL)
            {
                free(f->nodes[i].transitions);
            }
            if (f->nodes[i].name != NULL)
            {
                free(f->nodes[i].name);
            }
        }
        free(f->nodes);
    }
    if (f->events != NULL)
    {
        for (size_t i = 0; i < f->num_of_events; i++)
            if (f->events[i].name != NULL)
            {
                free(f->events[i].name);
            }
        free(f->events);
    }
    free(f);
    SM_DEBUG_MESSAGE("sm_fsm at [addr:%p] successfully destroyed", f);
    *sm = NULL;
}

sm_fsm *sm_fsm_create(const char *fsm_json, sm_directory *dir)
{
    sm_fsm *fsm;
    char er[80];
    if ((fsm = malloc(sizeof(sm_fsm))) == NULL)
    {
        SM_REPORT_MESSAGE(SM_LOG_ERR, "malloc() failed");
        return NULL;
    }
    // fsm->this = fsm;
    // fsm->ref = &fsm->this;
    // fsm->type = type;
    // fsm->class = class;
    fsm->name = NULL;
    fsm->nodes = NULL;
    fsm->events = NULL;

    jsmn_parser parser;
    jsmn_init(&parser);
    int ta_size = jsmn_parse(&parser, fsm_json, strlen(fsm_json), NULL, 0);
    if (ta_size <= 0)
    {
        free(fsm);
        SM_REPORT_ERROR("jsmn_parse() = %i", ta_size);
        return NULL;
    }

    jsmntok_t *tokens = NULL;
    if ((tokens = calloc(ta_size, sizeof(jsmntok_t))) == NULL)
    {
        free(fsm);
        SM_REPORT_MESSAGE(SM_LOG_ERR, "calloc() failed");
        return NULL;
    }

    jsmn_init(&parser);
    int num_of_tokens = jsmn_parse(&parser, fsm_json, strlen(fsm_json), tokens, ta_size);
    if (num_of_tokens <= 0)
    {
        SM_REPORT_ERROR("jsmn_parse() = %i", num_of_tokens);
        return cleanup(fsm, tokens);
    }

    typedef enum
    {
        START,
        SM_S,
        SM_OBJ,
        SM_O,
        //          SM_TYPE,
        //          SM_CLASS,
        //          SM_INIITAL,
        //          SM_FINAL,
        //          SM_APPLY_F,
        SM_NAME,
        NODES_A,
        NODE_OBJ,
        //              NEW_NODE,
        NODE_O,
        N_NAME,
        N_TYPE,
        N_ID,
        TRANSITIONS_A,
        TRANSITION_OBJ,
        //                NEW_TRANSITION,
        TRANSITION_O,
        T_TYPE,
        T_TARGET,
        T_REF,
        T_EVENT,
        T_SET,
        EVENTS_A,
        EVENT_OBJ,
        //              NEW_EVENT,
        EVENT_O,
        E_ID,
        E_NAME,
        FF,
        RW,
        SKIP,
        STOP
    } parse_state;

    parse_state state = START;
    parse_state stack = STOP;
    //    char *fsm_name;
    size_t skip_tokens = 0;
    size_t sm_c, non_c, n_c, not_c, t_c, noe_c, e_c;
    sm_c = non_c = n_c = not_c = t_c = noe_c = e_c = 0;
    bool tn_f, ref_f;
    //    tn_f = ref_f = false;
    //    char *fsm_initial;
    //    char *fsm_final;
    //    char *fsm_apply_f;
    sm_fsm_transition *node_transitions;
    uint16_t node_idx = 0;
    uint16_t node_id;
    size_t transition_idx = 0;
    size_t event_idx = 0;
    char *node_name;
    sm_fsm_node_type node_type;
    //    sm_fsm_transition *transition;
    char *transition_ref_name;
    //    size_t cnt;
    int mark;
    size_t i;
    for (i = 0; state != STOP && i < num_of_tokens; i++)
    {
        jsmntok_t *t = &tokens[i];

        if (t->start == -1 || t->end == -1)
        {
            SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "FSM JSON incorrect boundaries", "Malformed FSM JSON");
            return cleanup(fsm, tokens);
        }

        switch (state)
        {

        case START:
            if (t->type != JSMN_OBJECT)
            {
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "FSM JSON root element must be object", "Malformed FSM JSON");
                return cleanup(fsm, tokens);
            }
            state = SM_S;
            break;

        case SM_S:
            if (t->type != JSMN_STRING)
            {
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "FSM JSON object tag must string", "Malformed FSM JSON");
                return cleanup(fsm, tokens);
            }
            if ((fsm->name = tostr(fsm_json, t)) == NULL)
            {
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "Error parsing FSM name", "Malformed FSM JSON");
                return cleanup(fsm, tokens);
            }
            dir = sm_directory_set(dir, fsm->name, (void *)fsm);
            state = SM_OBJ;
            break;

        case SM_OBJ:
            if (t->type == JSMN_OBJECT)
            {
                sm_c = t->size;
                if (sm_c < 2)
                {
                    SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "FSM JSON header incomplete", "Malformed FSM JSON");
                    return cleanup(fsm, tokens);
                }
            }
            else
            {
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "FSM JSON root element must be object", "Malformed FSM JSON");
                return cleanup(fsm, tokens);
            }
            state = SM_O;
            break;

        case SM_O:
            if (sm_c == 0)
            {
                state = STOP;
                break;
            }
            else if (t->type != JSMN_STRING)
            {
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "FSM JSON element tag must be string", "Malformed FSM JSON");
                return cleanup(fsm, tokens);
            }
            else if (streq(fsm_json, t, "nodes"))
            {
                sm_c--;
                state = NODES_A;
                break;
            }
            else if (streq(fsm_json, t, "events"))
            {
                sm_c--;
                state = EVENTS_A;
                break;
            }
            else
            {
                sm_c--;
                state = FF;
                stack = SM_O;
                mark = t->end;
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_WARNING, "Unrecognized FSM JSON element", "Extending FSM JSON");
                break;
            }

        case NODES_A:
            if (t->type == JSMN_ARRAY)
            {
                non_c = t->size;
                state = NODE_OBJ;
                if ((fsm->nodes = calloc(non_c, sizeof(sm_fsm_node))) == NULL)
                {
                    SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "Error allocating memory for FSM nodes", "Malformed FSM JSON");
                    return cleanup(fsm, tokens);
                }
                fsm->num_of_nodes = non_c;
                break;
            }
            else
            {
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "FSM nodes must be stored in array", "Malformed FSM JSON");
                return cleanup(fsm, tokens);
            }

        case NODE_OBJ:
            if (t->type != JSMN_OBJECT)
            {
                //                    printf("\nJSMN token type : %d", t->type);
                //                    printf("\nJSMN token : %s", tostr(fsm_json, t));
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "FSM JSON node root element must be object", "Malformed FSM JSON");
                return cleanup(fsm, tokens);
            }
            n_c = t->size;
            if (n_c < 4)
            {
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "FSM JSON node header incomplete", "Malformed FSM JSON");
                return cleanup(fsm, tokens);
            }
            state = NODE_O;
            break;

        case NODE_O:
            if (n_c == 0)
            {
                if ((fsm->nodes[node_idx].name = malloc(strlen(node_name))) == NULL)
                {
                    SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "Error allocating memory for FSM node", "Malformed FSM JSON");
                    return cleanup(fsm, tokens);
                }
                strcpy(fsm->nodes[node_idx].name, node_name);
                free(node_name);
                fsm->nodes[node_idx].transitions = node_transitions;
                fsm->nodes[node_idx].id = node_id;
                if (node_type == SM_INITIAL)
                {
                    fsm->initial = node_id;
                }
                else if (node_type == SM_FINAL)
                {
                    fsm->final = node_id;
                }
                fsm->nodes[node_idx].type = node_type;
                // node_idx++;
                if (node_idx++ >= fsm->num_of_nodes)
                {
                    SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "Error parsing FSM nodes", "Malformed FSM JSON");
                    return cleanup(fsm, tokens);
                }
                if (--non_c == 0)
                {
                    stack = SM_O;
                    state = RW;
                    break;
                }
                // non_c--;
                stack = NODE_OBJ;
                state = RW;
                break;
            }
            else if (t->type != JSMN_STRING)
            {
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "FSM JSON node element tag must be string", "Malformed FSM JSON");
                return cleanup(fsm, tokens);
            }
            else if (streq(fsm_json, t, "type"))
            {
                n_c--;
                state = N_TYPE;
                break;
            }
            else if (streq(fsm_json, t, "name"))
            {
                n_c--;
                state = N_NAME;
                break;
            }
            else if (streq(fsm_json, t, "id"))
            {
                n_c--;
                state = N_ID;
                break;
            }
            else if (streq(fsm_json, t, "transitions"))
            {
                n_c--;
                state = TRANSITIONS_A;
                break;
            }
            else
            {
                n_c--;
                state = FF;
                stack = NODE_O;
                mark = t->end;
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_WARNING, "Unrecognized FSM JSON node element", "Extending FSM JSON");
                break;
            }

        case N_TYPE:
            if (t->type == JSMN_STRING)
            {
                if (streq(fsm_json, t, "state"))
                {
                    node_type = SM_STATE;
                }
                else if (streq(fsm_json, t, "process"))
                {
                    node_type = SM_PROCESS;
                }
                else if (streq(fsm_json, t, "initial"))
                {
                    node_type = SM_INITIAL;
                }
                else if (streq(fsm_json, t, "final"))
                {
                    node_type = SM_FINAL;
                }
                else
                {
                    char *cc = tostr(fsm_json, t);
                    printf("\ntoken = %s\n", cc);
                    fflush(NULL);
                    free(cc);
                    SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "Unknown FSM node type", "Malformed FSM JSON");
                    return cleanup(fsm, tokens);
                }
            }
            else
            {
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "FSM node type must be string", "Malformed FSM JSON");
                return cleanup(fsm, tokens);
            }
            state = NODE_O;
            break;

        case N_NAME:
            if (t->type == JSMN_STRING)
            {
                if ((node_name = tostr(fsm_json, t)) == NULL)
                {
                    SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "Error parsing node name", "Malformed FSM JSON");
                    return cleanup(fsm, tokens);
                }
            }
            else
            {
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "FSM node name must be string", "Malformed FSM JSON");
                return cleanup(fsm, tokens);
            }
            state = NODE_O;
            break;

        case N_ID:
            if ((node_id = toint(fsm_json, t)) < 0)
            {
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "Error parsing node id", "Malformed FSM JSON");
                return cleanup(fsm, tokens);
            }
            state = NODE_O;
            break;

        case TRANSITIONS_A:
            if (t->type == JSMN_ARRAY)
            {
                not_c = t->size;
                state = TRANSITION_OBJ;
                if ((node_transitions = calloc(not_c, sizeof(sm_fsm_transition))) == NULL)
                {
                    SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "Error allocating memory for FSM transitions", "Malformed FSM JSON");
                    return cleanup(fsm, tokens);
                }
                fsm->nodes[node_idx].num_of_transitions = not_c;
                transition_idx = 0;
                break;
            }
            else
            {
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "FSM node transitions must be stored in array", "Malformed FSM JSON");
                return cleanup(fsm, tokens);
            }

        case TRANSITION_OBJ:
            if (t->type != JSMN_OBJECT)
            {
                //                    printf("\nJSMN token type : %d", t->type);
                //                    printf("\nJSMN token : %s", tostr(fsm_json, t));
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "FSM JSON transition root element must be object", "Malformed FSM JSON");
                return cleanup(fsm, tokens);
            }
            t_c = t->size;
            if (t_c < 5)
            {
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "FSM JSON transition header incomplete", "Malformed FSM JSON");
                return cleanup(fsm, tokens);
            }
            tn_f = ref_f = false;
            state = TRANSITION_O;
            break;

        case TRANSITION_O:
            if (t_c == 0)
            {
                transition_idx++;
                if (!ref_f)
                {
                    SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "FSM JSON transition missing mandatory application element", "Malformed FSM JSON");
                    return cleanup(fsm, tokens);
                }
                if (!tn_f)
                {
                    SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "FSM JSON transition missing mandatory target node id element", "Malformed FSM JSON");
                    return cleanup(fsm, tokens);
                }
                if (--not_c == 0)
                {
                    // everything is already written to node_transitions
                    // need to sort and copy to fsm->nodes[node_id]->transitions
                    // adding omega if not present in JSON (++not)
                    stack = NODE_O;
                    state = RW;
                    break;
                }
                // not_c--;
                stack = TRANSITION_OBJ;
                state = RW;
                break;
            }
            else if (t->type != JSMN_STRING)
            {
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "FSM JSON transition element tag must be string", "Malformed FSM JSON");
                return cleanup(fsm, tokens);
            }
            else if (streq(fsm_json, t, "type"))
            {
                t_c--;
                state = T_TYPE;
                break;
            }
            else if (streq(fsm_json, t, "targetNode"))
            {
                t_c--;
                tn_f = true;
                state = T_TARGET;
                break;
            }
            else if (streq(fsm_json, t, "reference"))
            {
                t_c--;
                ref_f = true;
                state = T_REF;
                break;
            }
            else if (streq(fsm_json, t, "appliedOnEvent"))
            {
                t_c--;
                state = T_EVENT;
                break;
            }
            else if (streq(fsm_json, t, "setEventId"))
            {
                t_c--;
                state = T_SET;
                break;
            }
            else
            {
                t_c--;
                state = FF;
                stack = TRANSITION_O;
                mark = t->end;
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_WARNING, "Unrecognized FSM JSON transition element", "Extending FSM JSON");
                break;
            }

        case T_TARGET:
            if ((node_transitions[transition_idx].targetNode = toint(fsm_json, t)) < 0)
            {
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "Error parsing transition target node id", "Malformed FSM JSON");
                return cleanup(fsm, tokens);
            }
            state = TRANSITION_O;
            break;

        case T_TYPE:
            if (t->type == JSMN_STRING)
            {
                if (streq(fsm_json, t, "regular"))
                {
                    node_transitions[transition_idx].type = SM_REGULAR;
                }
                else if (streq(fsm_json, t, "cascade"))
                {
                    node_transitions[transition_idx].type = SM_CASCADE;
                }
                else if (streq(fsm_json, t, "hierarchical"))
                {
                    node_transitions[transition_idx].type = SM_HIERARCHICAL;
                }
                else if (streq(fsm_json, t, "async"))
                {
                    node_transitions[transition_idx].type = SM_ASYNC;
                }
                else if (streq(fsm_json, t, "static"))
                {
                    node_transitions[transition_idx].type = SM_STATIC;
                }
                else
                {
                    SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "Unknown FSM transition type", "Malformed FSM JSON");
                    return cleanup(fsm, tokens);
                }
            }
            else
            {
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "FSM transition type must be string", "Malformed FSM JSON");
                return cleanup(fsm, tokens);
            }
            state = TRANSITION_O;
            break;

        case T_REF:
            if ((transition_ref_name = tostr(fsm_json, t)) == NULL)
            {
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "Error parsing transition reference name", "Malformed FSM JSON");
                return cleanup(fsm, tokens);
            }
            node_transitions[transition_idx].transitionRef = (void *)sm_directory_get_ref(dir, transition_ref_name);
            if (node_transitions[transition_idx].transitionRef == NULL)
            {
                sprintf(er, "Transition reference %s not found in the directory", transition_ref_name);
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, er, "Malformed FSM JSON");
                free(transition_ref_name);
                return cleanup(fsm, tokens);
            }
            free(transition_ref_name);
            state = TRANSITION_O;
            break;

        case T_EVENT:
            if ((node_transitions[transition_idx].appliedOnEvent = toint(fsm_json, t)) < 0)
            {
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "Error parsing transition event id", "Malformed FSM JSON");
                return cleanup(fsm, tokens);
            }
            state = TRANSITION_O;
            break;

        case T_SET:
            if ((node_transitions[transition_idx].setEventId = toint(fsm_json, t)) < 0)
            {
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "Error parsing set event id", "Malformed FSM JSON");
                return cleanup(fsm, tokens);
            }
            state = TRANSITION_O;
            break;

        case EVENTS_A:
            if (t->type == JSMN_ARRAY)
            {
                noe_c = t->size;
                state = EVENT_OBJ;
                if ((fsm->events = calloc(noe_c, sizeof(sm_fsm_event))) == NULL)
                {
                    SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "Error allocating memory for FSM event descriptors", "Malformed FSM JSON");
                    return cleanup(fsm, tokens);
                }
                fsm->num_of_events = noe_c;
                break;
            }
            else
            {
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "FSM event descriptors must be stored in array", "Malformed FSM JSON");
                return cleanup(fsm, tokens);
            }

        case EVENT_OBJ:
            if (t->type != JSMN_OBJECT)
            {
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "FSM JSON event root element must be object", "Malformed FSM JSON");
                return cleanup(fsm, tokens);
            }
            e_c = t->size;
            if (e_c < 2)
            {
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "FSM JSON event header incomplete", "Malformed FSM JSON");
                return cleanup(fsm, tokens);
            }
            state = EVENT_O;
            break;

        case EVENT_O:
            if (e_c == 0)
            {
                event_idx++;
                if (--noe_c == 0)
                {
                    stack = SM_O;
                    state = RW;
                    break;
                }
                // noe_c--;
                stack = EVENT_OBJ;
                state = RW;
                break;
            }
            else if (t->type != JSMN_STRING)
            {
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "FSM JSON transition element tag must be string", "Malformed FSM JSON");
                return cleanup(fsm, tokens);
            }
            else if (streq(fsm_json, t, "name"))
            {
                e_c--;
                state = E_NAME;
                break;
            }
            else if (streq(fsm_json, t, "id"))
            {
                e_c--;
                state = E_ID;
                break;
            }
            else
            {
                e_c--;
                state = FF;
                stack = EVENT_O;
                mark = t->end;
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_WARNING, "Unrecognized FSM JSON transition element", "Extending FSM JSON");
                break;
            }

        case E_ID:
            if ((fsm->events[event_idx].id = toint(fsm_json, t)) < 0)
            {
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "Error parsing event id", "Malformed FSM JSON");
                return cleanup(fsm, tokens);
            }
            state = EVENT_O;
            break;

        case E_NAME:
            if (t->type == JSMN_STRING)
            {
                // printf("\nevent idx = %ld, number of events = %ld", event_idx, noe_c);
                // fflush(NULL);
                if ((fsm->events[event_idx].name = tostr(fsm_json, t)) == NULL)
                {
                    SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "Error parsing event name", "Malformed FSM JSON");
                    return cleanup(fsm, tokens);
                }
            }
            else
            {
                SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "FSM event name must be string", "Malformed FSM JSON");
                return cleanup(fsm, tokens);
            }
            state = EVENT_O;
            break;

        case FF:
            if (t->start < mark)
            {
                state = FF;
                break;
            }
            else
            {
                state = stack;
                break;
            }

        case RW:
            state = stack;
            i -= 2;
            break;

        case SKIP:
            skip_tokens--;
            if (skip_tokens == 0)
                state = stack;
            break;

        default:
            SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "FSM JSON parser error", "parser invalid state");
            return cleanup(fsm, tokens);
        }
    }

    // post-processing
    if (state != STOP && i < num_of_tokens)
    {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "FSM JSON exit state != STOP", "Malformed FSM JSON");
        return cleanup(fsm, tokens);
    }
    SM_DEBUG_MESSAGE("sm_fsm at [addr:%p] successfully created", fsm);
    return fsm;
}

char *sm_fsm_to_string(sm_fsm *f, sm_directory *dir)
{
    extern char sm_buffer[];
    sm_buffer[0] = '\0';
    if (f == NULL)
        return "";
    char line[1024];
    sprintf(sm_buffer, "SM Name: %s\n", f->name);
    sprintf(line, "The number of nodes: %ld\n", f->num_of_nodes);
    strcat(sm_buffer, line);
    sprintf(line, "The final node: %d\n", f->final);
    strcat(sm_buffer, line);
    sprintf(line, "The initial node: %d\n", f->initial);
    strcat(sm_buffer, line);
    sprintf(line, "The nodes:\n");
    strcat(sm_buffer, line);
    for (size_t i = 0; i < f->num_of_nodes; i++)
    {
        sprintf(line, "  Node name: %s\n", f->nodes[i].name);
        strcat(sm_buffer, line);
        sprintf(line, "  Node d: %d\n", f->nodes[i].id);
        strcat(sm_buffer, line);
        sprintf(line, "  Node type: %s\n", sm_node_type_to_string[f->nodes[i].type]);
        strcat(sm_buffer, line);
        sprintf(line, "  The number of transitions: %ld\n", f->nodes[i].num_of_transitions);
        strcat(sm_buffer, line);
        sprintf(line, "  The transitions:\n");
        strcat(sm_buffer, line);
        for (size_t j = 0; j < f->nodes[i].num_of_transitions; j++)
        {
            sprintf(line, "    Transition # %ld\n", j);
            strcat(sm_buffer, line);
            sprintf(line, "    Transition type: %s\n", sm_transition_type_to_string[f->nodes[i].transitions[j].type]);
            strcat(sm_buffer, line);
            sprintf(line, "    Trigger event Id: %d\n", f->nodes[i].transitions[j].appliedOnEvent);
            strcat(sm_buffer, line);
            sprintf(line, "    Target node Id: %d\n", f->nodes[i].transitions[j].targetNode);
            strcat(sm_buffer, line);
            sprintf(line, "    (Re)set event Id: %d\n", f->nodes[i].transitions[j].setEventId);
            strcat(sm_buffer, line);
            sprintf(line, "    Transition reference: %s\n",
                    sm_directory_get_name(dir, *(void **)(f->nodes[i].transitions[j].transitionRef)));
            strcat(sm_buffer, line);
        }
    }
    sprintf(line, "The events:\n");
    strcat(sm_buffer, line);
    for (size_t i = 0; i < f->num_of_events; i++)
    {
        sprintf(line, "  Event name: %s\n", f->events[i].name);
        strcat(sm_buffer, line);
        sprintf(line, "  Event Id: %d\n", f->events[i].id);
        strcat(sm_buffer, line);
    }
    return sm_buffer;
}

int sm_fsm_get_initial_state(sm_fsm *f)
{
    if (f != NULL)
    {
        return f->initial;
    }
    else
    {
        return -1;
    }
}

//////////////// refactor

sm_fsm_node *sm_state_get_node(sm_state *s)
{
    sm_fsm *f = *(s->fsm);
    size_t i;
    for (i = 0; i < f->num_of_nodes && f->nodes[i].id != s->state_id; i++)
        ;
    if (i == f->num_of_nodes)
    {
        return NULL;
    }
    else
    {
        return &(f->nodes[i]);
    }
}

sm_fsm_transition *sm_state_get_transition(sm_event *e, sm_state *s)
{
    sm_fsm_node *n = sm_state_get_node(s);
    size_t i;
    for (i = 0; i < n->num_of_transitions && n->transitions[i].appliedOnEvent != e->event_id; i++)
        ;
    if (i == n->num_of_transitions)
    {
        for (i = 0; i < n->num_of_transitions && n->transitions[i].appliedOnEvent != 0; i++)
            ;
    }
    if (i < n->num_of_transitions)
    {
        return &(n->transitions[i]);
    }
    else
    {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "Cannot find transition in SM", "No default transition found");
        return NULL;
    }
}

static inline sm_fsm *cleanup(sm_fsm *fsm, jsmntok_t *tokens)
{
    sm_fsm_destroy(&fsm);
    free(tokens);
    return NULL;
}

static inline char *tostr(const char *f, jsmntok_t *t)
{
    char *js;
    size_t token_size = t->end - t->start;
    if ((js = malloc(token_size + 1)) == NULL)
        return NULL;
    strncpy(js, &f[t->start], token_size);
    js[token_size] = '\0';
    //    printf("\n==> tostr(fsm_jsmn, t) = %s", js);
    //    fflush(NULL);
    return js;
}

static inline bool streq(const char *js, jsmntok_t *t, const char *s)
{
    //    char *ts;
    //    if(strncmp(js + t->start, s, t->end - t->start) == 0
    //       && strlen(s) == (size_t) (t->end - t->start)) {
    //        ts = tostr(js, t);
    //        free(ts);
    //    }
    return (strncmp(js + t->start, s, t->end - t->start) == 0 && strlen(s) == (size_t)(t->end - t->start));
}

static inline int toint(const char *f, jsmntok_t *t)
{
    char *s = tostr(f, t);
    if (s == NULL)
        return -1;
    int i = atoi(s);
    free(s);
    return i;
}