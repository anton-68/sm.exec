/* SM.EXEC
   FSM
   (c) anton.bondarenko@gmail.com */

#ifndef SM_FSM_H
#define SM_FSM_H

#include <stdint.h>
#include "sm_sys.h"
#include "sm_app.h"
#include "sm_directory.h"
//#include "sm_event.h"
//#include "sm_state.h"
#include "sm_logger.h"

//DEPRECATED ??
//#define SM_FSM(S) (*(S)->fsm)
//#define SM_FSM_EVENT_ID(s, e) (e)->id >= SM_FSM(s)->num_of_nodes ? 0 : (e)->id

typedef enum sm_fsm_transition_type {
    SM_REGULAR,            // sm_app **     / char *app_name
    SM_CASCADE,            // sm_array **   / char *array_name
    SM_HIERARCHICAL,       // sm_array **   / char *array_name
    SM_ASYNC,              // sm_queue2 **  / char *queue2_name
    SM_STATIC              // sm_fsm **     / char *fsm_name
} sm_fsm_transition_type;

typedef enum sm_fsm_node_type {
    SM_PROCESS,
    SM_STATE,
    SM_INITIAL,
    SM_FINAL
} sm_fsm_node_type;

typedef struct sm_fsm_transition {
    void *transitionRef;
    SM_EVENT_ID appliedOnEvent;
    SM_EVENT_ID setEventId;
    SM_STATE_ID targetNode;
    sm_fsm_transition_type type;
} sm_fsm_transition;

typedef struct sm_fsm_node {
    char *name; 
    sm_fsm_transition *transitions;
    size_t num_of_transitions;
    SM_STATE_ID id;
    //SM_EVENT_ID default_event;
    sm_fsm_node_type type;
} sm_fsm_node;

typedef struct sm_fsm_event {
    char *name;
    SM_EVENT_ID id;
} sm_fsm_event;

typedef struct sm_fsm {
    char *name;
//    struct sm_fsm *this;  // Do we still need it
//    struct sm_fsm **ref;  // ??
    sm_fsm_node *nodes;
    size_t num_of_nodes;
    sm_fsm_event *events;
    size_t num_of_events;
    SM_STATE_ID initial;
    SM_STATE_ID final;
    //sm_fsm_type type;
} sm_fsm;

// Public methods
sm_fsm* sm_fsm_create(const char *fsm_json, sm_directory *dir);
void sm_fsm_free(sm_fsm *f);
char *sm_fsm_to_string(sm_fsm *f, sm_directory *dir);

#endif //SM_FSM_H
