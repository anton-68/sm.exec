/* SM.EXEC
   FSM
   (c) anton.bondarenko@gmail.com */

#ifndef SM_FSM_H
#define SM_FSM_H

#include <stdint.h> 
#include "sm_app.h"
#inckude "sm_event.h"
#include "sm_state.h"

// DEPRECATED [[
#define SM_FSM(S) (*(S)->fsm)
#define SM_FSM_EVENT_ID(s, e) (e)->id >= SM_FSM(s)->num_of_nodes ? 0 : (e)->id
// ]]  

typedef enum sm_fsm_type {
	SM_MEALY,
    SM_MOORE
} sm_fsm_type;

typedef enum sm_fsm_transition_type {
	SM_REGULAR, 			// sm_app *
    SM_CASCADE,				// state **
	SM_HIERARCHIC			// fsm **
} sm_fsm_transition_type;

typedef enum sm_fsm_node_type {
	SM_SWITCH,
    SM_STATE
} sm_fsm_node_type;

typedef struct sm_fsm_transition {
	char *name; 
	void *transitionRef;
	SM_EVENT_ID appliedOnEvent;
	SM_EVENT_ID setEventId;
	SM_STATE_ID targetState;
	sm_fsm_transition_type;
} sm_fsm_transition;

typedef struct sm_fsm_node {
	char *name; 
	sm_fsm_transition *transitions;
	size_t num_of_transitions;
	SM_STATE_ID id;
	sm_fsm_node_type type;
} sm_fsm_node;

typedef int (*sm_fsm_apply_f) (struct sm_state *, sm_event *); //??
typedef struct sm_fsm {
	struct sm_fsm *this;
	struct sm_fsm **ref;
	sm_fsm_node *nodes;
	size_t num_of_nodes;
	sm_fsm_type type;
	SM_STATE_ID initial;
	SM_STATE_ID final;
}

// Public methods
sm_fsm* sm_fsm_create(const char *fsm_json, sm_directory *dir, sm_fsm_type type/*, sm_fsm_class class*/);
void sm_fsm_free(sm_fsm *f);
char *sm_fsm_to_string(sm_fsm *f);

#endif //SM_EVENT_H
