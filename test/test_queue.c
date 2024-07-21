/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Queue module tests
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "../src/sm_queue.h"
#include "test_utils.h"

int main()
{
    // create empty not synchronized queue q0
    sm_queue *q0 = SM_QUEUE_CREATE_EMPTY(false);
    sm_print_queue(q0);
    sm_print_event(SM_QUEUE_TOP(q0));
    
    // enqueue event
    sm_event *e0 = sm_event_create(0, false, false, false, false);
    sm_queue_enqueue(e0, q0);
    sm_print_event(SM_QUEUE_TOP(q0));

    // repeat for different even types
    sm_queue_enqueue(sm_event_create(128, true, false, false, false), q0);
    strcpy(SM_EVENT_DATA(SM_QUEUE_TOP(q0)), "sm_event_create(128, true, false, false, false)");
    sm_queue_enqueue(sm_event_create(256, false, true, false, true), q0);
    strcpy(SM_EVENT_DATA(SM_QUEUE_TOP(q0)), "sm_event_create(256, false, true, false, true)");
    sm_queue_enqueue(sm_event_create(384, false, true, true, false), q0);
    strcpy(SM_EVENT_DATA(SM_QUEUE_TOP(q0)), "sm_event_create(384, false, true, true, false)");
    sm_print_queue(q0);

    e0 = sm_event_create(0, false, false, false, false); 
    sm_event *e1;
    for (int i = 0; i < 7; i++)
    {
        e1 = sm_event_create(1, false, false, false, false);
        e1->event_id = i;
        sm_event_chain(e0, e1);
    }
    sm_queue_enqueue(e0, q0);
    while(SM_QUEUE_SIZE(q0) > 0)
    {
        sm_print_event(sm_queue_dequeue(q0));
    }
    sm_print_queue(q0);

    // Destroy
    e0 = sm_event_create(128, true, false, false, false);
    sm_queue_enqueue(e0, q0);
    e0 = sm_event_create(256, false, true, false, true);
    sm_queue_enqueue(e0, q0);
    e0 = sm_event_create(384, false, true, true, false);
    sm_queue_enqueue(e0, q0);
    sm_print_queue(q0);
    sm_queue_destroy(q0);
    sm_print_queue(q0);
}