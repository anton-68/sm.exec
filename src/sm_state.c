/* SM.EXEC
   State
   anton.bondarenko@gmail.com */

#include <string.h> 		//memcmp
#include "sm_state.h"
#include "sm_queue.h"

#include <stdio.h>


/* sm_state */

//public methods

bool sm_state_key_match(sm_state *c, const void *key, size_t key_length){
	if(key_length != c->key_length)
		return false;
	else
		return memcmp(c->key, key, key_length);
}

int sm_state_set_key(sm_state *s, const void *key) {
	size_t key_length = strlen((char *)key);
	if(s->key == NULL) {
		if((s->key = malloc(key_length)) == NULL) {
        	REPORT(ERROR, "malloc()");
        	return EXIT_FAILURE;
    	}
	}	
	else {
		if(s->key_length != key_length) {
			free(s->key);
			if((s->key = malloc(key_length)) == NULL) {
        		REPORT(ERROR, "malloc()");
        		return EXIT_FAILURE;
			}
		}
	}
	memcpy(s->key, key, key_length + 1);
	s->key_length = key_length;
	return EXIT_SUCCESS;
}
		   
sm_state *sm_state_create(sm_fsm *f, size_t payload_size) {
    sm_state *s;
    if((s = malloc(sizeof(sm_state))) == NULL) {
        REPORT(ERROR, "malloc()");
        return s;
    }
	s->data_size = payload_size;
	char *str;
    if(s->data_size > 0) {
        if ((s->data = malloc(s->data_size)) == NULL) {
            REPORT(ERROR, "malloc()");
            free(s);
            return NULL;
        }
        else {
			str = (char *)(s->data);
			str[0] = '\0';
		}
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
	s->key = NULL;
	//str = (char *)(s->key);
	//str[0] = '\0';	
    s->key_length = 0;
    s->key_hash = 0;
	s->trace = NULL;
	return s;    
}

void sm_state_purge(sm_state *c) {
	if(c == NULL)
		return;
	c->key = NULL;
    c->key_length = 0;
    c->key_hash = 0;
	c->id = 0;
	c->fsm = NULL;
	c->next = NULL; // ??
	while(c->trace != NULL) {
		sm_event *ne = c->trace->next;
		c->trace->to_keep = false;
		sm_queue_enqueue(c->trace, c->trace->home);
		c->trace = ne;
	}
}

void sm_state_free(sm_state * c) {
	if(c == NULL)
		return;
	free(c->data);
	while(c->trace != NULL) {
		sm_event *ne = c->trace->next;
		c->trace->to_keep = false;
		sm_queue_enqueue(c->trace, c->trace->home);
		c->trace = ne;
	}
	free(c);
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
