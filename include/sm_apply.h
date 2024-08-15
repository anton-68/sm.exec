/* SM.EXEC
   Apply event function
   (c) anton.bondarenko@gmail.com */

#ifndef SM_APPLY_H
#define SM_APPLY_H

#include "sm_event.h"
#include "sm_state.h"

//#define SM_FSM(S) (*(S)->fsm)

void sm_apply_event(sm_event *e, sm_state *s); 

#endif //SM_APPLY_H
