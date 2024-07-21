/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Test utils
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "test_utils.h"

void sm_print_event(const sm_event *e)
{
    char buffer[SM_TEST_PRINT_BUFFER];
    sm_event_to_string(e, buffer);
    printf("Event\n=====\n%s\n", buffer);
    while (e != NULL && e->ctl.L)
    {
        e = e->next;
        sm_event_to_string(e, buffer);
        printf("\nLinked event\n=====\n%s\n\n", buffer);
    }
}

void sm_print_state(sm_state *s)
{
    char buffer[SM_TEST_PRINT_BUFFER];
    sm_state_to_string(s, buffer);
    printf("State\n=====\n%s\n", buffer);
    if (s != NULL && s->ctl.E) {
        sm_event *e = SM_STATE_EVENT_TRACE(s);
        do
        {
            sm_event_to_string(e, buffer);
            printf("Stack event\n===========\n%s\n", buffer);
        } while (e != NULL && (e = e->next) != NULL);
    }
}

void sm_print_queue(sm_queue *q)
{
    char buffer[SM_TEST_PRINT_BUFFER];
    sm_queue_to_string(q, buffer);
    printf("Queue\n=====\n%s\n", buffer);
}