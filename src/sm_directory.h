/* SM.EXEC
   Directory
   (c) anton.bondarenko@gmail.com */

#ifndef SM_DIRECTORY_H
#define SM_DIRECTORY_H

#include "sm_sys.h"

/* Name record */
typedef struct sm_directory_record {
    char *name;
    void *ptr;
    void **ref;
    struct sm_directory_record *prev;
    struct sm_directory_record *next;
} sm_directory_record;

/* Registry object */
typedef struct sm_directory {
    sm_directory_record *top;
} sm_directory;

// Public methods

sm_directory *sm_directory_create();
sm_directory *sm_directory_set(sm_directory *t, const char *name, void *ptr);
void **sm_directory_get_ref(sm_directory *t, const char *name);
char *sm_directory_get_name(sm_directory *t, void *ptr);
void sm_directory_remove(sm_directory *t, const char *name);
void sm_directory_free(sm_directory *t);
    
#endif //SM_DIRECTORY_H
