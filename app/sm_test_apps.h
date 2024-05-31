/* SM.EXEC 
   Test applications
   anton.bondarenko@gmail.com */

#ifndef SM_TEST_APPS_H
#define SM_TEST_APPS_H

#include "../src/sm_event.h"
#include "../src/sm_state.h"

// DEPRECATED
int sm_test_app1(sm_event *e);
int sm_test_app2(sm_event *e);
int sm_test_app3(sm_event *e);
int sm_test_app4(sm_event *e);
int sm_nope(sm_event *e);

int WAIT(sm_event *e, sm_state *s);
int QUE0(sm_event *e, sm_state *s);
int QUE1(sm_event *e, sm_state *s);
int NOAP(sm_event *e, sm_state *s);

#endif //SM_TEST_APPS_H