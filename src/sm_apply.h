/* SM.EXEC
   Apply event function
   (c) anton.bondarenko@gmail.com */

#ifndef SM_APPLY_H
#define SM_APPLY_H

#include "sm_event"
#include "sm_state"

// Public methods
void sm_apply_event(sm_state *s, sm_event *e); 

#endif //SM_APPLY_H