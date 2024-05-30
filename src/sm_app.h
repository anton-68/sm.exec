/* SM.EXEC
   Some system utilities and definintions
   (c) anton.bondarenko@gmail.com */

#ifndef SM_APP_H
#define SM_APP_H

#include "sm_sys.h"
#include "sm_event.h"

/* Application prototype */ 
typedef int (*sm_app) (sm_event *);

/* Application registry*/
typedef struct sm_app_table {
	char * name;
	sm_app app;
	sm_app *ref;
	struct sm_app_table *prev;
	struct sm_app_table *next;
} sm_app_table;

// Public methods

sm_app_table *sm_app_table_create();
sm_app_table *sm_app_table_set(sm_app_table *t, const char *name, sm_app app);
sm_app *sm_app_table_get_ref(sm_app_table *t, const char *name);
void sm_app_table_remove(sm_app_table *t, const char *name);

#endif //SM_APP_H