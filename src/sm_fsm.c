/* SM.EXEC
   FSM   
   anton.bondarenko@gmail.com */

#include <stdlib.h>			// malloc-free
#include <string.h>
#include <stdio.h>			// sprintf

#include "sm_sys.h"
#include "sm_fsm.h"
#include "../lib/jsmn/jsmn.h"
#include "../oam/logger.h"

/* sm_fsm */

// --- Private methods ---------------------------------------

static sm_fsm *cleanup(sm_fsm *fsm, jsmntok_t *tokens){
	sm_fsm_free(fsm);
	free(tokens);
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
	if (s == NULL)
		return -1;
	int i = atoi(s);
	free(s);
    return i;
}


// --- Public methods ----------------------------------------

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

sm_fsm* sm_fsm_create(const char *fsm_json, sm_directory *dir, sm_fsm_type type, sm_fsm_class class){
	sm_fsm *fsm;
	//?? char er[80];
	if((fsm = malloc(sizeof(sm_fsm))) == NULL) {
        REPORT(ERROR, "malloc()");
        return NULL;
    }
	fsm->this = fsm;
	fsm->ref = &fsm->this;
	fsm->type = type;
	fsm->class = class;
		
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
		  SM_S,
		  SM_OBJ,
		  SM_O,
		  SM_TYPE,
		  SM_CLASS,
		  SM_INIITAL,
		  SM_FINAL,
		  SM_APPLY_F,
		  SM_NAME,
		    NODES_A,
			NODE_OBJ,
			NEW_NODE,
			NODE_O,
			N_NAME,
			N_TYPE,
			TRANSITIONS_A,
		      NEW_TRANSITION,
			  TRANSITION_O,
			  T_NAME
		      T_TARGET,
		      T_APP,
		      T_EVENT,
		      T_SET,
		FF,
		SKIP,
		STOP
	} parse_state;
	
	parse_state state = START;
	parse_state stack = STOP;
	size_t skip_tokens = 0;
	size_t sm_c, non_c, n_c, not_c, t_c;
	sm_c = non_c = n_c = not_c = t_c = 0;
	bool tn_f, app_f;
	tn_f = app_f = false;
	char *fsm_initial;
	char *fsm_final;
	char *fsm_apply_f;
	sm_fsm_transition *node_transitions;
	SM_STATE_ID node_id = 1;
	char* node_name;
	sm_fsm_node_type node_type;
	sm_fsm_transition *transition;
	char *transition_app_name;
	size_t cnt;
	
	for (size_t i = 0; state != STOP && i < num_of_tokens; i++) {
		jsmntok_t *t = &tokens[i];
		
		if(t->start == -1 || t->end == -1) {
			SM_SYSLOG(SM_CORE, SM_LOG_ERR, "FSM JSON incorrect boundaries", "Malformed FSM JSON");
			return cleanup(fsm, tokens, parsed_nodes, parsed_transitions);
		}

		switch (state) {
				
			case START:
				if(t->type != JSMN_OBJECT) {
					SM_SYSLOG(SM_CORE, SM_LOG_ERR, "FSM JSON root element must be object", "Malformed FSM JSON");
					return cleanup(fsm, tokens);
				}
				state = SM_S;
				break;
				
			case SM_S:	
				if(t->type != JSMN_STRING) {
					SM_SYSLOG(SM_CORE, SM_LOG_ERR, "FSM JSON object tag must string", "Malformed FSM JSON");
					return cleanup(fsm, tokens);
				}
				if(!streq(fsm_json, t, "sm")){
					SM_SYSLOG(SM_CORE, SM_LOG_ERR, "FSM JSON object tag must be \'sm\'", "Malformed FSM JSON");
					return cleanup(fsm, tokens);
				}
				state = SM_OBJ;
				break;
				
			case SM_OBJ:
				if(t->type == JSMN_OBJECT) {
					sm_c = t->size;
					if(sm_c < 7) {
						SM_SYSLOG(SM_CORE, SM_LOG_ERR, "FSM JSON header incomplete", "Malformed FSM JSON");
						return cleanup(fsm, tokens);
					}
				}
				else {
					SM_SYSLOG(SM_CORE, SM_LOG_ERR, "FSM JSON root element must be object", "Malformed FSM JSON");
					return cleanup(fsm, tokens);
				}
				state = SM_O;
				break;
				
			case SM_O:
				if(sm_c == 0){
					state = STOP;
					break;
				} 
				else if (t->type != JSMN_STRING) {
					SM_SYSLOG(SM_CORE, SM_LOG_ERR, "FSM JSON element tag must be string", "Malformed FSM JSON");
					return cleanup(fsm, tokens);
				}
				else if(streq(fsm_json, t, "type")) {
					sm_c--;
					state = SM_TYPE;
					break;
				}
				else if(streq(fsm_json, t, "class")) {
					sm_c--;
					state = SM_CLASS;
					break;
				}
				else if(streq(fsm_json, t, "initialEvent")) {
					sm_c--;
					state = SM_INITIAL;
					break;
				}
				else if(streq(fsm_json, t, "finalEvent")) {
					sm_c--;
					state = SM_FINAL;
					break;
				}
				else if(streq(fsm_json, t, "applyEventMethod")) {
					sm_c--;
					state = SM_APPLY_F;
					break;
				}
				else if(streq(fsm_json, t, "name")) {
					sm_c--;
					state = SM_NAME;
					break;
				}
				else if(streq(fsm_json, t, "nodes")) {
					sm_c--;
					state = NODES_A;
					break;
				}
				else {
					sm_c--;
					state = FF;
					stack = SM_O;
					mark = t->end;
					SM_SYSLOG(SM_CORE, SM_LOG_WARNING, "Unrecognized FSM JSON element", "Extending FSM JSON");
					break;
				}
		
			case SM_TYPE:
				if(t->type == JSMN_STRING){
					if(streq(fsm_json, t, "mealy")) {
						fsm->type = SM_MEALY;
						
					}
					else if(streq(fsm_json, t, "moore")) {
						fsm->type = SM_MOORE;
						
					}
					else {
						SM_SYSLOG(SM_CORE, SM_LOG_ERR, "Unknown FSM type", "Malformed FSM JSON");
						return cleanup(fsm, tokens);
					}
				}
				else {
					SM_SYSLOG(SM_CORE, SM_LOG_ERR, "FSM type must be string", "Malformed FSM JSON");
					return cleanup(fsm, tokens);
				}
				state = SM_O;
				break;
					
			case SM_CLASS:
				if(t->type == JSMN_STRING){
					if(streq(fsm_json, t, "static")) {
						fsm->type = SM_STATIC;
						
					}
					else if(streq(fsm_json, t, "dynamic")) {
						fsm->type = SM_DYNAMIC;
						
					}
					else {
						SM_SYSLOG(SM_CORE, SM_LOG_ERR, "Unknown FSM class", "Malformed FSM JSON");
						return cleanup(fsm, tokens);
					}
				}
				else {
					SM_SYSLOG(SM_CORE, SM_LOG_ERR, "FSM class must be string", "Malformed FSM JSON");
					return cleanup(fsm, tokens);
				}
				state = SM_O;
				break;
				
			case SM_INITIAL:
				if(t->type == JSMN_STRING){
					if((fsm_initial = tostr(fsm_json, t)) == NULL) {
						SM_SYSLOG(SM_CORE, SM_LOG_ERR, "Error parsing intial state", "Malformed FSM JSON");
						return cleanup(fsm, tokens);
					}
				}
				else {
					SM_SYSLOG(SM_CORE, SM_LOG_ERR, "FSM initial event name must be string", "Malformed FSM JSON");
					return cleanup(fsm, tokens);
				}
				state = SM_O;
				break;
					
			case SM_FINAL:
				if(t->type == JSMN_STRING){
					if((fsm_fial = tostr(fsm_json, t)) == NULL) {
						SM_SYSLOG(SM_CORE, SM_LOG_ERR, "Error parsing final state", "Malformed FSM JSON");
						return cleanup(fsm, tokens);
					}
				}
				else {
					SM_SYSLOG(SM_CORE, SM_LOG_ERR, "FSM final event name must be string", "Malformed FSM JSON");
					return cleanup(fsm, tokens);
				}
				state = SM_O;
				break;
					
			case SM_APPLY_F:
				if(t->type == JSMN_STRING){
					if((fsm_apply_f = tostr(fsm_json, t)) == NULL) {
						SM_SYSLOG(SM_CORE, SM_LOG_ERR, "Error parsing apply function name", "Malformed FSM JSON");
						return cleanup(fsm, tokens);
					}
				}
				else {
					SM_SYSLOG(SM_CORE, SM_LOG_ERR, "FSM apply function name must be string", "Malformed FSM JSON");
					return cleanup(fsm, tokens);
				}
				fsm->sm_fsm_apply_f = sm_directory_get_ref(dir, fsm_apply_f);
				state = SM_O;
				break;
					
			case SM_NAME:
				if(t->type == JSMN_STRING) {
					if((fsm_name = tostr(fsm_json, t)) == NULL) {
						SM_SYSLOG(SM_CORE, SM_LOG_ERR, "Error parsing apply function name", "Malformed FSM JSON");
						return cleanup(fsm, tokens);
					}
					dir = sm_directory_set(dir, fsm_name, (void *)fsm);
				}
				else {
					SM_SYSLOG(SM_CORE, SM_LOG_ERR, "FSM name must be string", "Malformed FSM JSON");
					return cleanup(fsm, tokens);
				}									   
				state = SM_O;
				break;
					
			case NODES_A:
				if(t->type == JSMN_ARRAY) {
					non_c = t->size;
					state = NODE_OBJ
					if((fsm->nodes = calloc(non_c, sizeof(sm_fsm_node))) == NULL) {
        				SM_SYSLOG(SM_CORE, SM_LOG_ERR, "Error allocating memory for FSM nodes", "Malformed FSM JSON");
        				return cleanup(fsm, tokens);
    				}
					fsm->num_of_nodes = non_c;
					break;
				}
				else {
					SM_SYSLOG(SM_CORE, SM_LOG_ERR, "FSM nodes must be stored in array", "Malformed FSM JSON");
					return cleanup(fsm, tokens);
				}
				
			case NODE_OBJ:
				if(t->type != JSMN_OBJECT) {
					SM_SYSLOG(SM_CORE, SM_LOG_ERR, "FSM JSON node root element must be object", "Malformed FSM JSON");
					return cleanup(fsm, tokens);
				}
				state = NEW_NODE;
				break;
				
			case NEW_NODE:
				if(t->type == JSMN_OBJECT) {
					n_c = t->size;
					if(n_c < 3){
						SM_SYSLOG(SM_CORE, SM_LOG_ERR, "FSM JSON node header incomplete", "Malformed FSM JSON");
						return cleanup(fsm, tokens);
					}
				}
				else {
					SM_SYSLOG(SM_CORE, SM_LOG_ERR, "FSM JSON node element must be object", "Malformed FSM JSON");
					return cleanup(fsm, tokens);
				}
				state = NODE_O;
				break;	
				
			case NODE_O:
				if(n_c == 0) {
					if(strcmp(fsm_final, node_name) == 0) 
						cnt = fsm->num_of_nodes-1;
					else if(strcmp(fsm_initial, node_name) == 0)
						cnt = 0;
					else
						cnt = node_id++;
					if((fsm->nodes[cnt]->name = malloc(strlen(node_name))) == NULL){
						SM_SYSLOG(SM_CORE, SM_LOG_ERR, "Error allocating memory for FSM node", "Malformed FSM JSON");
        				return cleanup(fsm, tokens);
					}
					strcpy(fsm->nodes[cnt]->name, node_name);
					fsm->nodes[cnt]->transitions = node_transitions;
					fsm->nodes[cnt]->id = node_id;
					fsm->nodes[cnt]->type = node_type;
					if (node_id >= fsm->num_of_nodes) {
						SM_SYSLOG(SM_CORE, SM_LOG_ERR, "Error parsing FSM nodes", "Malformed FSM JSON");
        				return cleanup(fsm, tokens);
					}
					if(non_c == 0){
						state = SM_O;
						break;
					}
					state = NODE_O;
					break;
				}
				else if (t->type != JSMN_STRING) {
					SM_SYSLOG(SM_CORE, SM_LOG_ERR, "FSM JSON node element tag must be string", "Malformed FSM JSON");
					return cleanup(fsm, tokens);
				}
				else if(streq(fsm_json, t, "type")) {
					n_c--;
					state = N_TYPE;
					break;
				}
				else if(streq(fsm_json, t, "name")) {
					n_c--;
					state = N_NAME;
					break;
				}
				else if(streq(fsm_json, t, "node")) {
					if(n_c > 0) {
						SM_SYSLOG(SM_CORE, SM_LOG_ERR, "FSM JSON node incomplete", "Malformed FSM JSON");
						return cleanup(fsm, tokens);
					}
					non_c--;
					state = NEW_NODE;
					break;
				}
				else if(streq(fsm_json, t, "transitions")) {
					n_c--;
					state = TRANSITIONS_A;
					break;
				}
				else {
					n_c--;
					state = FF;
					stack = NODE_O;
					mark = t->end;
					SM_SYSLOG(SM_CORE, SM_LOG_WARNING, "Unrecognized FSM JSON node element", "Extending FSM JSON");
					break;	
				}
				
			case N_TYPE:
				if(t->type == JSMN_STRING){
					if(streq(fsm_json, t, "state")) {
						node_type = SM_STATE;
						
					}
					else if(streq(fsm_json, t, "switch")) {
						node_type = SM_SWITCH;
						
					}
					else {
						SM_SYSLOG(SM_CORE, SM_LOG_ERR, "Unknown FSM node type", "Malformed FSM JSON");
						return cleanup(fsm, tokens);
					}
				}
				else {
					SM_SYSLOG(SM_CORE, SM_LOG_ERR, "FSM node type must be string", "Malformed FSM JSON");
					return cleanup(fsm, tokens);
				}
				state = NODE_O;
				break;
				
			case N_NAME:
				if(t->type == JSMN_STRING) {
					if((node_name = tostr(fsm_json, t)) == NULL) {
						SM_SYSLOG(SM_CORE, SM_LOG_ERR, "Error parsing node name", "Malformed FSM JSON");
						return cleanup(fsm, tokens);
					}
				}
				else {
					SM_SYSLOG(SM_CORE, SM_LOG_ERR, "FSM name must be string", "Malformed FSM JSON");
					return cleanup(fsm, tokens);
				}									   
				state = NODE_O;
				break;
				
			case TRANSITIONS_A:	
				if(t->type == JSMN_ARRAY) {
					not_c = t->size;
					state = TRANSITION_OBJ
					if((node_transitions = calloc(not_c, sizeof(sm_fsm_transition))) == NULL) {
        				SM_SYSLOG(SM_CORE, SM_LOG_ERR, "Error allocating memory for FSM transitions", "Malformed FSM JSON");
        				return cleanup(fsm, tokens);
    				}
					fsm->nodes[node_id]->num_of_transitions = not_c;
					transition_id = 0;
					break;
				}
				else {
					SM_SYSLOG(SM_CORE, SM_LOG_ERR, "FSM node transitions must be stored in array", "Malformed FSM JSON");
					return cleanup(fsm, tokens);
				}
				
			case TRANSITION_OBJ:
				if(t->type != JSMN_OBJECT) {
					SM_SYSLOG(SM_CORE, SM_LOG_ERR, "FSM JSON transition root element must be object", "Malformed FSM JSON");
					return cleanup(fsm, tokens);
				}
				state = NEW_TRANSITION;
				break;	
				
			case NEW_TRANSITION:
				if(t->type == JSMN_OBJECT) {
					t_c = t->size;
					if(t_c < 7){
						SM_SYSLOG(SM_CORE, SM_LOG_ERR, "FSM JSON transition header incomplete", "Malformed FSM JSON");
						return cleanup(fsm, tokens);
					}
					tn_f = app_f = false;
				}
				else {
					SM_SYSLOG(SM_CORE, SM_LOG_ERR, "FSM JSON transition element must be object", "Malformed FSM JSON");
					return cleanup(fsm, tokens);
				}
				state = TRANSITION_O;
				break;		
				
			case TRANSITION_O:
				if(t_c == 0) {
					transition_id++;
					if (!app_f){
						SM_SYSLOG(SM_CORE, SM_LOG_ERR, "FSM JSON transition missing mandatory application element", "Malformed FSM JSON");
						return cleanup(fsm, tokens);
					}
					if (!tn_f){
						SM_SYSLOG(SM_CORE, SM_LOG_ERR, "FSM JSON transition missing mandatory target node id element", "Malformed FSM JSON");
						return cleanup(fsm, tokens);
					}
					if(not_c == 0){
						// everything is already written to node_transitions
						// need to sort and copy to fsm->nodes[node_id]->transitions 
						// adding omega if not present in JSON (++not)
						state = NODE_O;
						break;
					}
					state = TRANSITION_O;
					break;
				}
				else if (t->type != JSMN_STRING) {
					SM_SYSLOG(SM_CORE, SM_LOG_ERR, "FSM JSON transition element tag must be string", "Malformed FSM JSON");
					return cleanup(fsm, tokens);
				}
				else if(streq(fsm_json, t, "name")) {
					t_c--;
					state = T_NAME;
					break;
				}
				else if(streq(fsm_json, t, "targetNode")) {
					t_c--;
					tn_f = true;
					state = T_TYPE;
					break;
				}
				else if(streq(fsm_json, t, "invokeOnTransition")) {
					t_c--;
					state = T_APP;
					break;
				}
				else if(streq(fsm_json, t, "appliedOnEvent")) {
					t_c--;
					tn_f = true;
					state = T_EVENT;
					break;
				}
				else if(streq(fsm_json, t, "setEventId")) {
					t_c--;
					state = T_SET;
					break;
				}
				else if(streq(fsm_json, t, "transition")) {
					if(t_c > 0) {
						SM_SYSLOG(SM_CORE, SM_LOG_ERR, "FSM JSON node incomplete", "Malformed FSM JSON");
						return cleanup(fsm, tokens);
					}
					not_c--;
					state = NEW_TRANSITION;
					break;
				}
				else {
					t_c--;
					state = FF;
					stack = TRANSITION_O;
					mark = t->end;
					SM_SYSLOG(SM_CORE, SM_LOG_WARNING, "Unrecognized FSM JSON transition element", "Extending FSM JSON");
					break;	
				}			
			
			case T_NAME:
				if((node_transitions[transition_id].name = tostr(fsm_json, t)) == NULL) {
					SM_SYSLOG(SM_CORE, SM_LOG_ERR, "Error parsing transition name", "Malformed FSM JSON");
					return cleanup(fsm, tokens);
				}	
				state = TRANSITION_O;
				break;
				
			case T_TARGET:
				if((node_transitions[transition_id].targetState = toint(fsm_json, t)) < 0) {
					SM_SYSLOG(SM_CORE, SM_LOG_ERR, "Error parsing transition target node id", "Malformed FSM JSON");
					return cleanup(fsm, tokens);
				}
				state = TRANSITION_O;
				break;
				
			case T_APP:
				if((transition_app_name = tostr(fsm_json, t)) == NULL) {
					SM_SYSLOG(SM_CORE, SM_LOG_ERR, "Error parsing transition app name", "Malformed FSM JSON");
					return cleanup(fsm, tokens);
				}
				node_transitions[transition_id].invokeOnTransition = (sm_app *)sm_directory_get_ref(dir, transition_app_name)
				if(node_transitions[transition_id].invokeOnTransition == NULL){
					SM_SYSLOG(SM_CORE, SM_LOG_ERR, "Transition app not found in the directory", "Malformed FSM JSON");
					return cleanup(fsm, tokens);
				}
				state = TRANSITION_O;
				break;
				
			case T_EVENT:
				if((node_transitions[transition_id].setEventId = toint(fsm_json, t)) < 0) {
					SM_SYSLOG(SM_CORE, SM_LOG_ERR, "Error parsing transition event id", "Malformed FSM JSON");
					return cleanup(fsm, tokens);
				}
				state = TRANSITION_O;
				break;
				
			case T_SET:	
				if((node_transitions[transition_id].appliedOnEvent = toint(fsm_json, t)) < 0) {
					SM_SYSLOG(SM_CORE, SM_LOG_ERR, "Error parsing translated event id", "Malformed FSM JSON");
					return cleanup(fsm, tokens);
				}
				state = TRANSITION_O;
				break;
				
			case FF:
				if(t->start < mark) {
					state = FF;
					break;
				}
				else { 
					state = stack;
					break;
				}	
		
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
	
	// post-processing
	if() {
		SM_SYSLOG(SM_CORE, SM_LOG_ERR, "FSM JSON ...", "Malformed FSM JSON");
		return cleanup(fsm, tokens);
	}

	return fsm;
}	


char *sm_fsm_to_string(sm_fsm* f) {
/*	
	extern char sm_buffer[];
	sm_buffer[0] = '\0';
	if(f == NULL) 
		return "";
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
*/	
}
