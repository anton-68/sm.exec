/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Resource Adaptor class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "sm_adaptor.h"

sm_adaptor *sm_adaptor_create(sm_fsm **adaptor_fsm,
                              uint16_t service_id,
                              sm_queue *depot, 
                              uint32_t db_size)
{
    sm_adaptor *a;
    if (SM_UNLIKELY((a = malloc(sizeof(sm_adaptor))) == NULL))
    {
        SM_REPORT_MESSAGE(SM_LOG_ERR, "malloc() failed");
        return NULL;
    }
    memset(a, '\0', sizeof(sm_adaptor));
    a->state = sm_state_create(adaptor_fsm, db_size, NULL, true, false, true, false);
    if (SM_UNLIKELY(a->state == NULL))
    {
        free(a);
        SM_REPORT_MESSAGE(SM_LOG_ERR, "sm_state_create() failed");
        return NULL;
    }
    a->depot = depot;
    SM_DEBUG_MESSAGE("sm_adaptor [addr:%p] successfully created", a);
    return a;
}

void sm_adaptor_destroy(sm_adaptor **a)
{
    if (a != NULL && *a != NULL)
    {
        sm_destroy_state((*a)->state);
        sm_destroy_queue((*a)->depot);
        free(*a);
        *a = NULL;
        SM_DEBUG_MESSAGE("sm_adaptor at [addr:%p] successfully destroyed", *a);
    }
    else
    {
        SM_REPORT_MESSAGE(SM_LOG_WARNING, "an attempt to call destroy function for the NULL pointer");
        return;
    }
}
