/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Service Descriptor class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#ifndef SM_SERVICE_H
#define SM_SERVICE_H

#include "sm_sys.h"
#include "sm_fsm.h"
#include "sm_queue.h"
#include "sm_array.h"

typedef int (*sm_service_action)(sm_service *);

typedef struct sm_service
{
    sm_service_action deploy;
    sm_service_action start;
    sm_service_action stop;
    sm_state *state;
    sm_queue *depot;
    sm_array *array;
    struct sm_service *next;
} sm_service;

sm_service *sm_service_create(sm_fsm **service_fsm,
                              uint16_t service_id,
                              uint32_t db_size,
                              sm_queue *depot,
                              sm_array *array);
void sm_service_destroy(sm_service **s);

#endif // SM_SERVICE_H