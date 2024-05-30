/* SM.EXEC
   State
   anton.bondarenko@gmail.com */

#include <string.h> 		//memcmp
#include "sm_state.h"
#include "sm_queue.h"

#include <stdio.h>

/* sm_state */

//public methods

bool sm_state_key_match(sm_state *s, const void *key, size_t key_length){
	if(key_length != s->key_length)
		return false;
	else
		return memcmp(s->key, key, key_length);
}

int sm_state_set_key(sm_state *s, const void *key, size_t key_length) {
	memset(s->key, '\0', SM_STATE_HASH_KEYLEN);
	s->key_length = MIN(key_length, SM_STATE_HASH_KEYLEN);
	memcpy(s->key, key, MIN(key_length, s->key_length));
	return EXIT_SUCCESS;
}
	
sm_state *sm_state_create(sm_fsm *f, size_t payload_size) {
    sm_state *s;
    if((s = malloc(sizeof(sm_state))) == NULL) {
        REPORT(ERROR, "malloc()");
        return s;
    }
	s->data_size = payload_size;
    if(s->data_size > 0) {
        if ((s->data = malloc(s->data_size)) == NULL) {
            REPORT(ERROR, "malloc()");
            free(s);
            return NULL;
        }
        else
			memset(s->data, '\0', s->data_size);
    }
    else {
        s->data = NULL;
        s->data_size = 0;
    }
	s->fsm = f;	
	if(s->fsm != NULL)
		s->id = f->initial;
	else
		s->id = 0;
	s->next = NULL;
	if(SM_STATE_HASH_KEYLEN > 0) {
        if ((s->key = malloc(SM_STATE_HASH_KEYLEN)) == NULL) {
            REPORT(ERROR, "malloc()");
            free(s);
            return NULL;
        }
        else 
			memset(s->key, '\0', SM_STATE_HASH_KEYLEN);
    }
    else
        s->key = NULL;
    s->key_length = 0;
    s->key_hash = 0;
	s->trace = NULL;
	return s;    
}

void sm_state_purge(sm_state *s) {
	if(s == NULL)
		return;
	memset(s->key, '\0', SM_STATE_HASH_KEYLEN);
    s->key_length = 0;
    s->key_hash = 0;
	s->id = 0;
	s->fsm = NULL;
	s->next = NULL; // ??
	memset(s->data, '\0', s->data_size);
	while(s->trace != NULL) {
		sm_event *ne = s->trace->next;
		s->trace->to_keep = false;
		for (int stage = 0; stage < SM_NUM_OF_PRIORITY_STAGES; stage++)
			ne->priority[stage] = 0;
		sm_queue_enqueue(s->trace, s->trace->home);
		s->trace = ne;
	}
}

void sm_state_free(sm_state * s) {
	if(s == NULL)
		return;
	free(s->data);
	while(s->trace != NULL) {
		sm_event *ne = s->trace->next;
		s->trace->to_keep = false;
		sm_queue_enqueue(s->trace, s->trace->home);
		s->trace = ne;
	}
	free(s);
}

void sm_apply_event(sm_state *s, sm_event *e){
	do {
		if(s->fsm->type == SM_MEALY) {
			s->id = s->fsm->table[s->id][e->id].new_node;
			sm_app *a = s->fsm->table[s->id][e->id].action;
			if(a != NULL) (*a)(e);
		}
		else {
			sm_app *a = s->fsm->table[s->id][e->id].action;
			if(a != NULL) (*a)(e);
			s->id = s->fsm->table[s->id][e->id].new_node;
		}
	} while(s->fsm->nodes[s->id] == SM_JOINT);
}
