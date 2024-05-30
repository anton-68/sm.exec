/* SM.EXEC
   FSM   
   anton.bondarenko@gmail.com */

#include <stdlib.h>			// malloc-free
#include <string.h>
#include <stdio.h>			// sprintf

#include "sm_sys.h"
#include "sm_fsm.h"
#include "sm_app.h"
#include "jsmn.h"
#include "../oam/logger.h"

/* sm_fsm */

// Private types & objects

typedef struct node {
	SM_EVENT_ID id;
	sm_fsm_node_type type;
} node;

static node *parsed_nodes = NULL;

typedef enum event_type {
    REGULAR,
	DEFAULT
} event_type;

typedef struct event {
	SM_EVENT_ID id;
	event_type type;
} event;

static event* parsed_events = NULL;

typedef struct transition {
	SM_STATE_ID sid;
	SM_EVENT_ID eid;
	SM_STATE_ID nid;
	sm_app *action;
} transition;

static transition* parsed_transitions = NULL;

static bool *oflags = NULL;
static bool **tflags = NULL;

// Private methods

static sm_fsm *cleanup(sm_fsm *fsm, jsmntok_t *tokens){
	sm_fsm_free(fsm);
	free(tokens);
	if(parsed_nodes != NULL)
		free(parsed_nodes);
	if(parsed_events != NULL)
		free(parsed_events);
	if(parsed_transitions != NULL)
		free(parsed_transitions);
	if(oflags != NULL)
		free(oflags);
	if(tflags != NULL){
		for(int i = 0; i < fsm->num_of_nodes; i++)
			if(tflags[i] != NULL)
				free(tflags[i]);
		free(tflags);
	}
	return NULL;
}

static bool streq(const char *js, jsmntok_t *t, const char *s) {
    return (strncmp(js + t->start, s, t->end - t->start) == 0
            && strlen(s) == (size_t) (t->end - t->start));
}

static char *tostr(const char *f, jsmntok_t *t) {
	char *js;
	size_t token_size = t->end - t->start;
	if((js = malloc(token_size + 1)) == NULL) 
		return NULL;
	strncpy(js, &f[t->start], token_size);
    js[token_size] = '\0';
    return js;
}

static int toint(const char *f, jsmntok_t *t) {
	char *s = tostr(f, t);
	int i = atoi(s);
	free(s);
    return i;
}

// Public methods

void sm_fsm_free(sm_fsm *f) {
	if(f->nodes != NULL)
		free(f->nodes);
	if(f->table != NULL){
		for(size_t i = 0; i < f->num_of_events; i++)
			if(f->table[i] != NULL)
				free(f->table[i]);
		free(f->table);
	free(f);
	}
}

sm_fsm *sm_fsm_create(const char *fsm_json, sm_directory *at, sm_fsm_type t){
	sm_fsm *fsm;
	char er[80];
    if((fsm = malloc(sizeof(sm_fsm))) == NULL) {
        REPORT(ERROR, "malloc()");
        return NULL;
    }
	fsm->this = fsm;
	fsm->ref = &fsm->this;
	fsm->type = t;
	jsmn_parser parser;
	jsmn_init(&parser);
	int ta_size = jsmn_parse(&parser, fsm_json, strlen(fsm_json), NULL, 0);
	if (ta_size <= 0) {
		free(fsm);
		sprintf(er, "jsmn_parse() = %i", ta_size);
		REPORT(ERROR, er);
		return NULL;
	}
	
	jsmntok_t *tokens = NULL;
	if((tokens = calloc(ta_size, sizeof(jsmntok_t))) == NULL) {
		free(fsm);
		REPORT(ERROR, "calloc()");
		return NULL;
	}
	jsmn_init(&parser);
	int num_of_tokens = jsmn_parse(&parser, fsm_json, strlen(fsm_json), tokens, ta_size);			
	if (num_of_tokens <= 0) {
		sprintf(er, "jsmn_parse() = %i", num_of_tokens);
		REPORT(ERROR, er);
		return cleanup(fsm, tokens);
	}
	
	typedef enum {
		START,
		ARRAY_NAME,
		NODES_ARRAY, 
			NODE_ID, 
			NODE_TYPE,
		EVENTS_ARRAY,
			EVENT_ID,
			EVENT_TYPE,
		TRANSITIONS_ARRAY, 
			TRANSITION_STATE, 
			TRANSITION_EVENT, 
			TRANSITION_NEW_STATE,
			TRANSITION_ACTION,
		SKIP,
		STOP
	} parse_state;
	
	parse_state state = START;
	parse_state stack = STOP;
	size_t arrays = 3;
	size_t skip_tokens = 0;
	size_t idx, num_of_nodes, num_of_events, num_of_transitions; 
	idx = num_of_nodes = num_of_events = num_of_transitions = 0; 
	size_t non, noe, not;
	non = noe = not = 0;
	char *s;
	char *tok;
	
	for (size_t i = 0; state != STOP; i++) {
		jsmntok_t *t = &tokens[i];
		
		if(t->start == -1 || t->end == -1) {
			REPORT(ERROR, "fsm json boudaries");
			return cleanup(fsm, tokens);
		}

		switch (state) {
				
			case START:
				if(t->type != JSMN_OBJECT) {
					REPORT(ERROR, "fsm json root element must be object");
					return cleanup(fsm, tokens);
				}
				state = ARRAY_NAME;
				break;
				
			case ARRAY_NAME:
				if(t->type != JSMN_STRING) {
					REPORT(ERROR, "fsm array name must be string");
					return cleanup(fsm, tokens);
				}
				if(streq(fsm_json, t, "nodes"))
					state = NODES_ARRAY;
				else {
					if(streq(fsm_json, t, "events"))
						state = EVENTS_ARRAY;
					else {
						if (streq(fsm_json, t, "transitions")){
							state = TRANSITIONS_ARRAY;
						}
						else {
							REPORT(ERROR, "unknown fsm table name");
							return cleanup(fsm, tokens);
						}
					}
				}
				break;
				
			case NODES_ARRAY:
				if(t->type != JSMN_ARRAY) {
					REPORT(ERROR, "fsm json nodes must be array");
					return cleanup(fsm, tokens);
				}
				non = num_of_nodes = t->size;
				if((parsed_nodes = calloc(num_of_nodes, 
													  sizeof(node))) == NULL) {
					REPORT(ERROR, "calloc()");
					return cleanup(fsm, tokens);
				}
				idx = 0; 
				stack = NODE_ID; state = SKIP; skip_tokens = 2;
				break;
				
			case NODE_ID:
				if(t->type != JSMN_PRIMITIVE) {
					REPORT(ERROR, "fsm json node id must be integer");
					return cleanup(fsm, tokens);
				}
				parsed_nodes[idx].id = (SM_STATE_ID)toint(fsm_json, t);
				stack = NODE_TYPE; state = SKIP; skip_tokens = 1;
				break;
				
			case NODE_TYPE:
				if(t->type != JSMN_STRING) {
					REPORT(ERROR, "fsm json node type must be string");
					return cleanup(fsm, tokens);
				}
				s = tostr(fsm_json, t);
				if (s == NULL) {
					REPORT(ERROR, "malloc()");
					return cleanup(fsm, tokens); 
				}
				else if(!strcmp(s, "state"))
					parsed_nodes[idx].type = SM_STATE;
				else if(!strcmp(s, "initial"))
					parsed_nodes[idx].type = SM_INITIAL;		
				else if(!strcmp(s, "final"))
					parsed_nodes[idx].type = SM_FINAL;		
				else if(!strcmp(s, "joint"))
					parsed_nodes[idx].type = SM_JOINT;
				else {
					REPORT(ERROR, "fsm json unknown node type"); 
					return cleanup(fsm, tokens);
				}
				free(s);
				idx++;
				num_of_nodes--;
				if(num_of_nodes > 0)
					{state = SKIP; stack = NODE_ID; skip_tokens = 2;}
				else {
					arrays--;
					if(arrays > 0) 
						state = ARRAY_NAME;
					else 
						state = STOP;
				}
				break;
			
			case EVENTS_ARRAY:
				if(t->type != JSMN_ARRAY){
					REPORT(ERROR,  "fsm json event must be array");
					return cleanup(fsm, tokens);
				}
				noe = num_of_events = t->size;
				if((parsed_events = calloc(num_of_events, 
													  sizeof(event))) == NULL) {
					REPORT(ERROR, "calloc()");
					return cleanup(fsm, tokens);
				}
				idx = 0; 
				stack = EVENT_ID; state = SKIP; skip_tokens = 2;
				break;		
				
			case EVENT_ID:
				if(t->type != JSMN_PRIMITIVE){
					REPORT(ERROR, "fsm json event id must be integer");
					return cleanup(fsm, tokens);
				}
				parsed_events[idx].id = (SM_EVENT_ID)toint(fsm_json, t);
				stack = EVENT_TYPE; state = SKIP; skip_tokens = 1;
				break;	
				
			case EVENT_TYPE:
				if(t->type != JSMN_STRING){
					REPORT(ERROR, "fsm json node type must be string");
					return cleanup(fsm, tokens); 
				}
				s = tostr(fsm_json, t);
				if (s == NULL) {
					REPORT(ERROR, "malloc()");
					return cleanup(fsm, tokens);
				}
				else if(!strcmp(s, "regular"))
					parsed_events[idx].type = REGULAR;
				else if(!strcmp(s, "default"))
					parsed_events[idx].type = DEFAULT;
				else {
					REPORT(ERROR, "fsm json unknown event type");
					return cleanup(fsm, tokens);
				}
				free(s);
				idx++;
				num_of_events--;
				if(num_of_events > 0)
					{state = SKIP; stack = EVENT_ID; skip_tokens = 2;}
				else {
					arrays--;
					if(arrays > 0) 
						state = ARRAY_NAME;
					else 
						state = STOP;
				}
				break;	
	
			case TRANSITIONS_ARRAY:
				if(t->type != JSMN_ARRAY){
					REPORT(ERROR, "fsm json transitions must be array");
					return cleanup(fsm, tokens);
				}
				not = num_of_transitions = t->size;
				if((parsed_transitions = calloc(num_of_transitions, 
									   				sizeof(transition))) == NULL) {
					REPORT(ERROR, "calloc()");
					return cleanup(fsm, tokens);
				}
				idx = 0;   
				stack = TRANSITION_STATE; state = SKIP; skip_tokens = 2;
				break;
				
			case TRANSITION_STATE:
				if(t->type != JSMN_PRIMITIVE){
					REPORT(ERROR, "json state id must be integer");
					return cleanup(fsm, tokens);
				}
				parsed_transitions[idx].sid = (SM_STATE_ID)toint(fsm_json, t);
				stack = TRANSITION_EVENT; state = SKIP; skip_tokens = 1;
				break;
				
			case TRANSITION_EVENT:
				if(t->type != JSMN_PRIMITIVE) {
					REPORT(ERROR, "json event id must be integer");
					return cleanup(fsm, tokens);
				}
				parsed_transitions[idx].eid = (SM_EVENT_ID)toint(fsm_json, t);
				stack = TRANSITION_NEW_STATE; state = SKIP; skip_tokens = 1;
				break;	
				
			case TRANSITION_NEW_STATE:
				if(t->type != JSMN_PRIMITIVE){
					REPORT(ERROR,"json event id must be integer"); 
					return cleanup(fsm, tokens);
				}
				parsed_transitions[idx].nid = (SM_STATE_ID)toint(fsm_json, t);
				stack = TRANSITION_ACTION; state = SKIP; skip_tokens = 1;
				break;	
				
			case TRANSITION_ACTION:
				
				if(t->type == JSMN_PRIMITIVE) {
					tok = tostr(fsm_json, t);
					if(!strcmp(tok, "null")) {
						parsed_transitions[idx].action = NULL;
						free(tok);
					}
					else {
						REPORT(ERROR, "json app name must be string or null");
						return cleanup(fsm, tokens);
					}
				}
				else {
					if(t->type == JSMN_STRING) {
						tok = tostr(fsm_json, t);
						if(tok == NULL) {
							REPORT(ERROR, "malloc()");
							return cleanup(fsm, tokens);
						}
						parsed_transitions[idx].action = 
									(sm_app *)sm_directory_get_ref(at, tok);
						free(tok);
					}
					else {
						REPORT(ERROR, "json app name must be string or null");
						return cleanup(fsm, tokens);
					}
				}
				idx++;
				num_of_transitions--;
				if(num_of_transitions > 0)
					{state = SKIP; stack = TRANSITION_STATE; skip_tokens = 2;}
				else {
					arrays--;
					if(arrays > 0) 
						state = ARRAY_NAME;
					else 
						state = STOP;
				}
				break;
				
			case SKIP:
				skip_tokens--;
				if(skip_tokens == 0)
					state = stack;
				break;	
				
			default:
				REPORT(ERROR, "fsm json parser invalid state");
				return cleanup(fsm, tokens);

		}
	}
	
	// max node id & nodes
	size_t max_node_id = 0;
	for(int i = 0; i < non; i++) {
		if(parsed_nodes[i].id < 0) {
			REPORT(ERROR, "node id must be non-negative");
			return cleanup(fsm, tokens);
		}
		if(parsed_nodes[i].id > max_node_id)
			max_node_id = parsed_nodes[i].id;
		if(parsed_nodes[i].type == SM_INITIAL)
			fsm->initial = parsed_nodes[i].id;
		if(parsed_nodes[i].type == SM_FINAL)
			fsm->final = parsed_nodes[i].id;
	}
	fsm->num_of_nodes = max_node_id + 1;
	if((fsm->nodes = calloc(fsm->num_of_nodes, sizeof(sm_fsm_node))) == NULL) {
		REPORT(ERROR, "calloc()");
		return cleanup(fsm, tokens);
	}
	for(int i = 0; i < non; i++)
		fsm->nodes[parsed_nodes[i].id] = parsed_nodes[i].type;
	
	// max event id & events
	bool omega_set = false;
	size_t max_event_id = 0;
	//size_t omegas = 0;
	event *pe;
	for(int i = 0; i < noe; i++) {
		pe = &parsed_events[i];
		if(pe->id < 0) {
			REPORT(ERROR, "event id must be non-negative");
			return cleanup(fsm, tokens);
		}
		if(pe->id > max_event_id)
			max_event_id = pe->id;
		if(pe->type == DEFAULT) {
			if(!omega_set) {
				fsm->omega = pe->id;
				omega_set = true;
			}
			else {
				sprintf(er, "duplicate omega event definition: eid = %lu, type = %d", 
						pe->id, pe->type);
				REPORT(ERROR, er);
				return cleanup(fsm, tokens);
			}
		}
	}
	fsm->num_of_events = max_event_id + 1;
	// allocate flags
	if((tflags = calloc(fsm->num_of_nodes, sizeof(bool *))) == NULL){
		REPORT(ERROR, "calloc()");
		return cleanup(fsm, tokens);
	}
	for(int i = 0; i < fsm->num_of_events; i++)
		if((tflags[i] = calloc(fsm->num_of_events, sizeof(bool))) == NULL) {
			REPORT(ERROR, "calloc()");
			return cleanup(fsm, tokens);
		}
	if((oflags = calloc(fsm->num_of_nodes, sizeof(bool))) == NULL) {
		REPORT(ERROR, "calloc()");
		return cleanup(fsm, tokens);
	}
	// allocate and fill in fsm table
	if((fsm->table = calloc(fsm->num_of_nodes, sizeof(sm_fsm_transition *))) == NULL) {
		REPORT(ERROR, "calloc()");
		return cleanup(fsm, tokens);
	}
	for(int i = 0; i < fsm->num_of_nodes; i++)
		if((fsm->table[i] = calloc(fsm->num_of_events, sizeof(sm_fsm_transition))) == NULL) {
			REPORT(ERROR, "calloc()");
			return cleanup(fsm, tokens);
		}
	transition *pt; 
	for(int i = 0; i < not; i++) {
		pt = &parsed_transitions[i];
		if(pt->eid == fsm->omega) {
			if(oflags[pt->sid]) {
				sprintf(er, "duplicate omega transition definition: sid = %lu, eid = %lu",
						pt->sid, pt->eid);
				REPORT(ERROR, er);
				return cleanup(fsm, tokens);
			}
			else
				oflags[pt->eid] = true;
			for (int j = 0; j < noe; j++) {
				pe = &parsed_events[j];
				fsm->table[pt->sid][pe->id].new_node = pt->nid;
				fsm->table[pt->sid][pe->id].action = pt->action;
			}
		}
	}
	for(int i = 0; i < not; i++) {
		pt = &parsed_transitions[i];
		if(pt->eid != fsm->omega) {
			if(tflags[pt->sid][pt->eid]) {
				sprintf(er, "duplicate transition definition: sid = %lu, eid = %lu",
						pt->sid, pt->eid);
				REPORT(ERROR, er);
				return cleanup(fsm, tokens);
			}
			else
				tflags[pt->sid][pt->eid] = true;
			fsm->table[pt->sid][pt->eid].new_node = pt->nid;
			fsm->table[pt->sid][pt->eid].action = pt->action;
		}
	}
	free(tokens);
	free(parsed_nodes);
	free(parsed_events);
	free(parsed_transitions);
	free(oflags);
	for(int i = 0; i < non; i++)
		free(tflags[i]);
	free(tflags);
	return fsm;
}	

/* fsm pretty print */

char *sm_fsm_to_string(sm_fsm* f) {
	extern char sm_buffer[];
	sm_buffer[0] = '\0';
//	extern char *sm_buffer = sm_buffer;
	if(f == NULL) 
		return "";
//	char sm_buffer[] = "\n";
//	if((sm_buffer = malloc(SM_sm_buffer_BUF_LEN)) == NULL) {
//		REPORT(ERROR, "malloc()");
//		exit(0);
//	}

	char line[1024];
	char *node_type[] = {"undefined", "state", "initial", "final", "joint"};
	sprintf(line, "max number of node :  %lu\n", f->num_of_nodes - 1);
	strcat(sm_buffer, line);
    sprintf(line, "max number of event :  %lu\n", f->num_of_events - 1);
	strcat(sm_buffer, line);
	sprintf(line, "initial state Id :  %lu\n", f->initial);
	strcat(sm_buffer, line);
	sprintf(line, "final state Id :  %lu\n", f->final);
	strcat(sm_buffer, line);
	sprintf(line, "default event Id :  %lu\n", f->omega);
	strcat(sm_buffer, line);
	sprintf(line, "nodes :\n");
	strcat(sm_buffer, line);
	for(size_t i = 0; i < f->num_of_nodes; i++){
		sprintf(line, "node Id: %lu, node type: %s\n", i, node_type[f->nodes[i]]);
		strcat(sm_buffer, line);
	}
	sprintf(line, "transition function :\n");	
	strcat(sm_buffer, line);
	for(size_t i = 0; i < f->num_of_nodes; i++){
		sprintf(line, "node Id: %lu, transitions:", i);
		strcat(sm_buffer, line);
		for(size_t j = 0; j < f->num_of_events; j++){
			sprintf(line, " %lu", f->table[i][j].new_node);
			strcat(sm_buffer, line);
		}
		sprintf(line, "\n");
		strcat(sm_buffer, line);
	}
	sprintf(line, "sm_buffer function (actions) :\n");	
	strcat(sm_buffer, line);
	
	for(size_t i = 0; i < f->num_of_nodes; i++){
		sprintf(line, "node Id: %lu, actions:", i);
		strcat(sm_buffer, line);
		for(size_t j = 0; j < f->num_of_events; j++){
			void *ptr = (void *)f->table[i][j].action;
			sprintf(line, " %p", ptr == NULL ? NULL : (void *)*((sm_app *)ptr));
			strcat(sm_buffer, line);
		}
		sprintf(line, "\n");	
		strcat(sm_buffer, line);
	}
	return sm_buffer;
}

// DEPRECATED[
/* FSM registry */

// Private methods

static sm_fsm_table *find_record(sm_fsm_table *t, const char *name) {
	sm_fsm_table *r = t;
	while (r != NULL && strcmp(r->name, name)){
		r = r->next;
	}
	return r;
}

// Public methods

sm_fsm_table *sm_fsm_table_create() { return NULL; }

sm_fsm_table *sm_fsm_table_set(sm_fsm_table *t, const char *name, sm_fsm *fsm) {
	sm_fsm_table *r;
	if(t == NULL)
		r = NULL;
	else 
		r = find_record(t, name);
	if (r == NULL) {
		if((r = malloc(sizeof(sm_fsm_table))) == NULL) {
        	REPORT(ERROR, "malloc()");
        	return NULL;
		}
		if((r->name = malloc(strlen(name))) == NULL) {
        	REPORT(ERROR, "malloc()");
			free(r);
			return NULL;
    	}
		strcpy(r->name, name);
		r->ref = &(r->fsm);
		r->fsm = fsm;
		r->prev = NULL;
		r->next = t;
		if(t != NULL)
			r->next->prev = r;
		t = r;
    }
	else {
		char *r_n = r->name;
		if((r->name = malloc(strlen(name))) == NULL) {
        	REPORT(ERROR, "malloc()");
			r->name = r_n;
			return NULL;
    	}
		free(r_n);
		strcpy(r->name, name);
		r->fsm = fsm;
	}
	return t;		
}

sm_fsm **sm_fsm_table_get_ref(sm_fsm_table *t, const char *name) {	
	sm_fsm_table *tr = find_record(t, name);
	if (tr == NULL)
		return NULL;
	else
		return tr->ref;
}

void sm_fsm_table_remove(sm_fsm_table *t, const char *name) {
	sm_fsm_table *tr = find_record(t, name);
	if (tr != NULL) {
		tr->prev->next = tr->next;
		tr->next->prev = tr->prev;
		free(tr->name);
		free(tr);
	}
}

void sm_fsm_table_free(sm_fsm_table *t) {
	sm_fsm_table *tmp;
	while(t != NULL) {
		tmp = t->next;
		free(t);
		t = tmp;
	}
}
// ]