/* SM.EXEC
   State
   anton.bondarenko@gmail.com */

#include <string.h> 		//memcmp
#include "sm_state.h"
#include "sm_queue.h"




/* sm_state */

//public methods

bool sm_state_key_match(sm_state *c, void *const key, size_t key_length){
	if(key_length != c->key_length)
		return false;
	else
		return memcmp(c->key, key, key_length);
}

sm_state *sm_state_create(size_t payload_size) {
    sm_state *s;
    if((s = malloc(sizeof(sm_state))) == NULL) {
        REPORT(ERROR, "malloc()");
        return s;
    }      
    if(payload_size > 0) {
        if ((s->data = malloc(payload_size)) == NULL) {
            REPORT(ERROR, "malloc()");
            free(s);
            return NULL;
        }
        else
            s->data_size = payload_size;
    }
    else {
        s->data = NULL;
        s->data_size = 0;
    }
	s->id = 0;
	s->fsm = NULL;
	s->next = NULL;
	s->key = NULL;
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
		(*(s->fsm->table[s->id][e->id].action))(e);
		s->id = s->fsm->table[s->id][e->id].new_node;		
	} while(s->fsm->nodes[s->id] == SM_JOINT);
}
