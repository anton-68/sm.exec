/* SM.EXEC
   Apply event function
   (c) anton.bondarenko@gmail.com */

#include "sm_apply.h"
/*
#include <stdint.h>     // uint32_t
#include "sm_sys.h"
#include "sm_app.h"
#include "sm_fsm.h"
#include "sm_event"
#include "sm_state"
#include "../oam/logger.h"
*/

// --- Apply event -------------------------------------------

// Static methods

static size_t find_tr_idx(sm_state *s, sm_event *e) {
	size_t l = 0;
	size_t h = FSM(s)->nodes[s->id]->num_of_transitions - 1;
	size_t c;
	if(e->id > h)
		return 0; //
	while(l != h) {
		c = FSM(s)->nodes[s->id]->transitions[(l + h)/2]->appliedOnEvent;
		else if(e->id < c) {
			h = c;
		}
		else if(e->id > c) {
			if(h - l > 1) {
				l = c;
			}
			else { // h - l == 1 
				if(FSM(s)->nodes[s->id]->transitions[h]->appliedOnEvent == e->id) {
					return h;
				}
				else {
					return 0;
				}
			}
		}
		else {
			return (l + h)/2;
		}
	}
	return 0; // not found
}

// Public methods
void sm_apply_event(sm_state *s, sm_event *e){

	size_t tr_idx;
	SM_STATE_ID target;
	sm_app *app;
	
	
	do {
		tr_idx = find_tr_idx(s, e);
		target = SM_FSM(s)->nodes[s->id]->transitions[tr_idx]->targetState;
		app = SM_FSM(s)->nodes[s->id]->transitions[tr_idx]->invokeOnTransition;
		if(SM_FSM(s)->type == SM_MEALY) {
			s->id = target;
			if(SM_FSM(s)->nodes[s->id]->transitions[tr_idx]->setEventId != 0)
				e->id = SM_FSM(s)->nodes[s->id]->transitions[tr_idx]->setEventId;
			if(a != NULL) (*app)(e, s);;
		}
		else {
			if(a != NULL) (*app)(e, s);
			tr_idx = find_tr_idx(s, e);
			target = SM_FSM(s)->nodes[s->id]->transitions[tr_idx]->targetState;
			s->id = target;
			if(SM_FSM(s)->nodes[s->id]->transitions[tr_idx]->setEventId != 0)
				e->id = SM_FSM(s)->nodes[s->id]->transitions[tr_idx]->setEventId;
		}
	} while(SM_FSM(s)->nodes[s->id] == SM_SWITCH);


if(tx->state->id == (*tx->state->fsm)->final) 
				if(tx->state->home != NULL)
					sm_array_release_state(tx->state->home, tx->state);


}


