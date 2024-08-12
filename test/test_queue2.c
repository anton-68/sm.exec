/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Bipriority queue module tests
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "test_utils.h"

int main()
{
    printf("Basic operations: \n\n");
    sm_queue2 *q0 = sm_queue2_create();
    sm_print_queue2(q0);
    sm_print_event(sm_queue2_get(q0));
    sm_print_event(sm_queue2_get_high(q0));
    sm_event *e0 = sm_event_create(0, false, false, false, false);
    SM_QUEUE2_ENQUEUE(e0, q0);
    sm_print_event(sm_queue2_get(q0));
    sm_print_event(sm_queue2_get_high(q0));
    e0 = sm_dequeue2(q0);
    SM_QUEUE2_ENQUEUE_HIGH(e0, q0);
    sm_print_event(sm_queue2_get(q0));
    sm_print_event(sm_queue2_get_high(q0));
    e0 = sm_dequeue2(q0);
    SM_EVENT_DESTROY(e0);
    printf("\nPriorities (w/o locking): \n\n"); 
    for (int i = 0; i < 8; i++)
    {
        e0 = sm_event_create(1, false, false, false, false);
        e0->event_id = i * 2;
        SM_QUEUE2_ENQUEUE(e0, q0);
        e0 = sm_event_create(1, false, false, false, false);
        e0->event_id = i * 2 + 1;
        SM_QUEUE2_ENQUEUE_HIGH(e0, q0);
    }
    sm_print_queue2(q0);
    while (!sm_queue2_is_empty(q0))
    {
        sm_print_event(sm_dequeue2(q0));
    }
    sm_print_queue2(q0);
    printf("\nPriorities (with locking): \n\n");
    for (int i = 0; i < 8; i++)
    {
        e0 = sm_event_create(1, false, false, false, false);
        e0->event_id = i * 2;
        SM_QUEUE2_LOCK_ENQUEUE(e0, q0);
        e0 = sm_event_create(1, false, false, false, false);
        e0->event_id = i * 2 + 1;
        SM_QUEUE2_LOCK_ENQUEUE_HIGH(e0, q0);
    }
    sm_print_queue2(q0);
    while (!sm_queue2_is_empty(q0))
    {
        sm_print_event(sm_lock_dequeue2(q0));
    }
    SM_QUEUE2_DESTROY(q0);
    sm_print_queue2(q0);
}