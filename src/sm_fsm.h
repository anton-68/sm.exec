/* SM.EXEC
   FSM
   (c) anton.bondarenko@gmail.com */

#ifndef SM_FSM_H
#define SM_FSM_H

#include <stdint.h>     // uint32_t
#include "sm_sys.h"
#include "sm_app.h"

/* sm_fsm */

typedef enum sm_fsm_node_type {
	SM_UNDEFINED,
    SM_STATE,
	SM_INITIAL,
	SM_FINAL,
    SM_JOINT
} sm_fsm_node_type;

typedef enum sm_fsm_type {
	SM_MEALY,
    SM_MOORE
} sm_fsm_type;

typedef struct sm_fsm_node{
	SM_STATE_ID id;
	sm_fsm_node_type type;
} sm_fsm_node;

typedef struct sm_fsm_table {
	sm_app *action;
	SM_STATE_ID new_node;
} sm_fsm_table;

typedef struct sm_fsm {
	size_t num_of_nodes;
	size_t num_of_events;
	sm_fsm_node_type *nodes;  	// dim = num of nodes
	sm_fsm_table **table;		// dim = num of events * num of transitions	
	SM_STATE_ID initial;
	SM_STATE_ID final;
	SM_EVENT_ID omega;
	sm_fsm_type type;
} sm_fsm;

// Public methods

sm_fsm* sm_fsm_create(const char *f, sm_app_table *at, sm_fsm_type t);
void sm_fsm_free(sm_fsm *f);
//SM_STATE_ID sm_get_initial_state(sm_fsm *f);  // deprecated
char *sm_fsm_to_string(sm_fsm* f);

#endif //SM_EVENT_H
