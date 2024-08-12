/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
FSM executor descriptor class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#ifndef SM_EXEC_H
#define SM_EXEC_H

#include "sm_sys.h" 
#include "sm_queue2.h"
#include "sm_fsm.h"
#include "sm_apply.h"

#define SM_TX_STACK_POINTER(tx) (sm_state **)((char *)(tx)->data + (tx)->data_size /*- sizeof(sm_state **) */) 

struct sm_tx;
typedef struct sm_exec {
    sm_directory *dir;
    size_t data_size;
    void *data;
} sm_exec;

sm_exec *sm_exec_create(size_t s, sm_directory *dir);
void sm_exec_destroy(sm_exec **exec);
void sm_exec_shutdown(sm_exec **exec);
int sm_exec_to_string(sm_exec *e, char *buffer);

#endif //SM_EXEC_H 
