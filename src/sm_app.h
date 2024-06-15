/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Application function prototype and application table class
NOTE: The app_table class is deprecated, use the sm_directory class instead.
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#ifndef SM_APP_H
#define SM_APP_H

#include "sm_sys.h"
#include "sm_event.h"

/* Application prototype */ 
struct sm_state;
typedef int (*sm_app) (sm_event *, struct sm_state *);

// DEPRECATED [
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
void sm_app_table_free(sm_app_table *t);
// ] DEPRECATED

#endif //SM_APP_H
