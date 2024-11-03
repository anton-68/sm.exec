/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Resource Adaptor class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#ifndef SM_ADAPTOR_H
#define SM_ADAPTOR_H

#include "sm_sys.h"
#include "sm_fsm.h"
#include "sm_queue.h"

typedef int (*sm_adaptor_action)(sm_adaptor *);

typedef struct sm_adaptor
{
    sm_adaptor_action deploy;
    sm_adaptor_action start;
    sm_adaptor_action stop;
    sm_state *state;
    sm_queue *depot;
    struct sm_adaptor *next;
} sm_adaptor;

sm_adaptor *sm_adaptor_create(sm_fsm **adaptor_fsm,
                              uint16_t service_id,
                              sm_queue *depot,
                              uint32_t db_size);
void sm_adaptor_destroy(sm_adaptor **a);

#endif //SM_ADAPTOR_H