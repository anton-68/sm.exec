/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
FSM executor descriptor class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "sm_exec.h"

sm_exec *sm_exec_create(size_t size, sm_directory *dir)
{
    sm_exec *exec;
    if ((exec = malloc(sizeof(sm_exec))) == NULL)
    {
        SM_REPORT_MESSAGE(SM_LOG_ERR, "malloc() failed");
        return NULL;
    }
    memset(exec, '\0', sizeof(sm_exec));
    exec->data_size = size;
    if ((exec->data = malloc(exec->data_size)) == NULL)
    {
        SM_REPORT_MESSAGE(SM_LOG_ERR, "malloc() failed");
        free(exec);
        return NULL;
    }
    memset(exec->data, '\0', exec->data_size);
    exec->dir = dir;
    SM_DEBUG_MESSAGE("sm_exec at [addr:%p] successfully created", exec);
    return exec;
}

void sm_exec_destroy(sm_exec **exec)
{
    free((*exec)->data);
    free(*exec);
    SM_DEBUG_MESSAGE("sm_exec at [addr:%p] successfully destroyed", *exec);
    *exec = NULL;
}

void sm_exec_shutdown(sm_exec **exec)
{
    //sm_directory_purge((*exec)->dir);
    sm_exec_destroy(exec);
    SM_DEBUG_MESSAGE("system successfully shut down");
}

int sm_exec_to_string(sm_exec *e, char *buffer)
{
    char *s = buffer;
    if (SM_UNLIKELY(e == NULL))
    {
        s += sprintf(s, "NULL\n");
    }
    else
    {
        s += sprintf(s, "address: %p\n", e);
        s += sprintf(s, "data block size: %lu\n", e->data_size);
        s += sprintf(s, "data block address: %p\n", e->data);
        s += sprintf(s, "directory address: %p\n", e->dir);
    }
    return (int)((char *)s - (char *)buffer);
}