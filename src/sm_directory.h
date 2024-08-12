/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Directory [mock] class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#ifndef SM_DIRECTORY_H
#define SM_DIRECTORY_H

#include "sm_logger.h"

/* Name record */
typedef struct sm_directory_record
{
    char *name;
    void *ptr;
    void **ref;
    struct sm_directory_record *prev;
    struct sm_directory_record *next;
} sm_directory_record;

/* Registry object */
typedef struct sm_directory
{
    sm_directory_record *top;
} sm_directory;

sm_directory *sm_directory_create();
sm_directory *sm_directory_set(sm_directory *t, const char *name, void *ptr);
void **sm_directory_get_ref(sm_directory *t, const char *name);
char *sm_directory_get_name(sm_directory *t, void *ptr);
void sm_directory_remove(sm_directory *t, const char *name);
void sm_directory_destroy(sm_directory **d);
// void sm_directory_purge(sm_directory **t);
int sm_directory_to_string(sm_directory *d, char *buffer);

#endif // SM_DIRECTORY_H
