/* SM.EXEC 
   Test applications
   anton.bondarenko@gmail.com */

#ifndef SM_TEST_APPS_H
#define SM_TEST_APPS_H

#ifndef SM_DEBUG
#define SM_DEBUG
#endif

#include "../src/sm_event.h"
#include "../src/sm_state.h"
#include "../oam/logger.h"

// Traffic SM7 (Golly)

#define SM_ABS(X) (X) < 0 ? -(X) : (X)
#define SM_MAX(X, Y) (X) < (Y) ? (Y) : (X)
#define SM_X(E) ((int *)((E)->data))[0]
#define SM_Y(E) ((int *)((E)->data))[1]
#define SM_H(E) ((int *)((E)->data))[2]

typedef enum sm_trsim_event_type {
	SM_TRSIM_OMEGA,
	SM_TRSIM_BCME,
	SM_TRSIM_STEP1,
    SM_TRSIM_STEP2,
	SM_TRSIM_STEP3,
	SM_TRSIM_PLAN,
	SM_TRSIM_ERROR
} sm_trsim_event_type;

int trsim_check_neighbours(sm_event *e, sm_state *s);
int trsim_switch_fsm(sm_event *e, sm_state *s);
int trsim_brushfire(sm_event *e, sm_state *s);
int trsim_plan(sm_event *e, sm_state *s);
int trsim_pop_fsm(sm_event *e, sm_state *s);
int trsim_noap(sm_event *e, sm_state *s);

#endif //SM_TEST_APPS_H