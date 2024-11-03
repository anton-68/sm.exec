/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Service Descriptor class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "sm_service.h"

sm_service *sm_service_create(sm_fsm **service_fsm,
                              uint16_t service_id,
                              uint32_t db_size,
                              sm_queue *depot,
                              sm_array *array)
{
    sm_service *s;
    if (SM_UNLIKELY((s = malloc(sizeof(sm_service))) == NULL))
    {
        SM_REPORT_MESSAGE(SM_LOG_ERR, "malloc() failed");
        return NULL;
    }
    memset(s, '\0', sizeof(sm_service));
    s->state = sm_state_create(service_fsm, db_size, NULL, true, false, true, false);
    if (SM_UNLIKELY(s->state == NULL))
    {
        free(s);
        SM_REPORT_MESSAGE(SM_LOG_ERR, "sm_state_create() failed");
        return NULL;
    }
    s->depot = depot;
    s->array = array;
    SM_DEBUG_MESSAGE("sm_service [addr:%p] successfully created", s);
    return s;
}

void sm_adaptor_destroy(sm_service **s)
{
    if (s != NULL && *s != NULL)
    {
        sm_destroy_state((*s)->state);
        sm_destroy_queue((*s)->depot);
        sm_destroy_array((*s)->array);
        free(*s);
        *s = NULL;
        SM_DEBUG_MESSAGE("sm_service at [addr:%p] successfully destroyed", *s);
    }
    else
    {
        SM_REPORT_MESSAGE(SM_LOG_WARNING, "an attempt to call destroy function for the NULL pointer");
        return;
    }
}
