/* SM.EXEC
   Applications
   (c) anton.bondarenko@gmail.com */

#ifndef SM_APP_H
#define SM_APP_H

#include "sm_event.h"
 
struct sm_state;
typedef int (*sm_app) (sm_event *, struct sm_state *);

void *sm_app_loadlib(const char *fn);
sm_app sm_app_lookup(void * lib, const char *name);

#endif //SM_APP_H