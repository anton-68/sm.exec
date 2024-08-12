/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Priority queue module tests
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "test_utils.h"

int main()
{
    printf("\nPriory queue (w/o locking): \n\n");
    sm_pqueue *q0 = sm_pqueue_create(100, false);
    sm_print_pqueue(q0);
    sm_event *e0;
    for (int i = 0; i < 7; i++)
    {
        e0 = sm_event_create(1, false, false, true, false);
        e0->event_id = i;
        e0->service_id = 0;
        SM_EVENT_PRIORITY(e0)[0] = rand();
        SM_EVENT_PRIORITY(e0)[1] = rand();
        sm_pqueue_enqueue(q0, &e0);
    }
    sm_print_pqueue(q0);
    while (SM_PQUEUE_SIZE(q0) > 0)
    {
        e0 = sm_pqueue_dequeue(q0);
        sm_print_event(e0);
    }
    sm_print_pqueue(q0);

    printf("\nPriory queue (with locking): \n\n");
    sm_print_pqueue(q0);
    for (int i = 0; i < 7; i++)
    {
        e0 = sm_event_create(1, false, false, true, false);
        e0->event_id = i;
        e0->service_id = 1;
        SM_EVENT_PRIORITY(e0)
        [0] = rand();
        SM_EVENT_PRIORITY(e0)
        [1] = rand();
        sm_pqueue_enqueue(q0, &e0);
    }
    sm_print_pqueue(q0);
    while (SM_PQUEUE_SIZE(q0) > 0)
    {
        e0 = sm_pqueue_dequeue(q0);
        sm_print_event(e0);
    }
    sm_print_pqueue(q0);

    printf("\nPriory queue (w/o locking, 'opposite'): \n\n");
    sm_print_pqueue(q0);
    for (int i = 0; i < 7; i++)
    {
        e0 = sm_event_create(1, false, false, true, false);
        e0->event_id = i;
        e0->service_id = 1;
        SM_EVENT_PRIORITY(e0)
        [0] = i;
        SM_EVENT_PRIORITY(e0)
        [1] = 6 - i;
        sm_pqueue_enqueue(q0, &e0);
    }
    sm_print_pqueue(q0);
    while (SM_PQUEUE_SIZE(q0) > 0)
    {
        e0 = sm_pqueue_dequeue(q0);
        sm_print_event(e0);
    }
    sm_print_pqueue(q0);

    printf("\nPriory queue (w/o locking, 'thin'): \n\n");
    sm_print_pqueue(q0);
    for (int i = 0; i < 7; i++)
    {
        e0 = sm_event_create(1, false, false, true, false);
        e0->event_id = i;
        e0->service_id = 1;
        SM_EVENT_PRIORITY(e0)
        [0] = 0;
        SM_EVENT_PRIORITY(e0)
        [1] = i;
        sm_pqueue_enqueue(q0, &e0);
    }
    sm_print_pqueue(q0);
    while (SM_PQUEUE_SIZE(q0) > 0)
    {
        e0 = sm_pqueue_dequeue(q0);
        sm_print_event(e0);
    }
    sm_print_pqueue(q0);
}