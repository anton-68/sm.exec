/* SM.EXEC
   State
   anton.bondarenko@gmail.com */

#ifndef SM_STATE_H
#define SM_STATE_H

#include <stdlib.h>
#include "sm_event.h"
#include "sm_app.h"
#include "sm_fsm.h"

// DEPRECATED:
#define SM_STATE_FSM(S) (*(S)->fsm)
#define SM_STATE_EVENT_ID(s, e) (e)->id >= SM_STATE_FSM(s)->num_of_nodes ? SM_STATE_FSM(s)->omega : (e)->id

	
	/* DEPOT */	
	/// stack_size
	/// stack
	/// table_size
	/// table

	/// state_data size
	/// default_fsm_ref

	/// hash_size
	/// hash_mask
	/// hash_function
	/// last_hash_value;

typedef struct sm_state {
	
//  struct sm_state *next; ???
	uint32_t depot;
	struct {
		unsigned int id				: 16;
		unsigned int data_offset	:  8; // in x64 words
		unsigned int lock			:  1;
		unsigned int hash_key_flag	:  1;
		unsigned int fsm_ref_flag	:  1;
		unsigned int trace_flag		:  1;
		unsigned int /* reserved */	:  0;
	} ctl;	
	/// hash_key
	/// fsm_ref
	/// trace
} sm_state;

/* Life cycle */
sm_state *sm_state_create(sm_fsm **f, size_t payload_size);
void sm_state_purge(sm_state *c);
void sm_state_free(sm_state *c);
/* Hash key */
sm_hash_key *sm_event_hash_key_ptr(sm_event *e);


// DEPRECATED [
bool sm_state_key_match(sm_state *c, const void *key, size_t key_length);
int sm_state_set_key(sm_state *c, const void *key, size_t key_length);
// ]
//void sm_apply_event(sm_state *s, sm_event *e); // DEPRECATED: backward compatibility

#endif //SM_STATE_H 