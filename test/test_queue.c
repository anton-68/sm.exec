/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Queue module tests
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../src/sm_queue.h"

int main()
{
    char b[1024];
    
    // create empty unsynchronized queue q0
    sm_queue *q0 = SM_QUEUE_CREATE_EMPTY(false);
    
    // print q0
    sm_queue_to_string(q0, b);
    printf("Queue\n=====\n%s\n", b);

    // top q0
    sm_event *e0 = sm_queue_top(q0);
    printf("Top of empty queue: %p\n", e0);
    
    // create event
    e0 = sm_event_create(0, false, false, false, false);
    
    // enqueue
    sm_queue_enqueue(e0, q0);
    
    // print q0
    sm_queue_to_string(q0, b);
    printf("Queue 1\n=====\n%s\n", b);
    e0 = sm_queue_top(q0);
    sm_event_to_string(e0, b);
    printf("Top event\n=====\ntype: %u\n%s\n", e0->type, b);

    // repeat 3 times for different even types
    e0 = sm_event_create(128, true, false, false, false);
    strcpy(SM_EVENT_DATA(e0), "sm_event_create(128, true, false, false, false)");
    sm_queue_enqueue(e0, q0);
    sm_queue_to_string(q0, b);
    printf("Queue 2\n=====\n%s\n", b);
    e0 = sm_queue_top(q0);
    sm_event_to_string(e0, b);
    printf("Top event\n=====\ntype: %u\n%s\n", e0->type, b);

    e0 = sm_event_create(256, false, true, false, true);
    strcpy(SM_EVENT_DATA(e0), "sm_event_create(256, false, true, true, false)");
    sm_queue_enqueue(e0, q0);
    sm_queue_to_string(q0, b);
    printf("Queue 3\n=====\n%s\n", b);
    e0 = sm_queue_top(q0);
    sm_event_to_string(e0, b);
    printf("Top event\n=====\ntype: %u\n%s\n", e0->type, b);

    e0 = sm_event_create(384, false, true, true, false);
    strcpy(SM_EVENT_DATA(e0), "sm_event_create(384, false, true, true, false)");
    sm_queue_enqueue(e0, q0);
    sm_queue_to_string(q0, b);
    printf("Queue 4\n=====\n%s\n", b);
    e0 = sm_queue_top(q0);
    sm_event_to_string(e0, b);
    printf("Top event\n=====\ntype: %u\n%s\n", e0->type, b);
    
    // Purge q0
    while (sm_queue_size(q0) > 0)
    {
        e0 = sm_queue_dequeue(q0);
        sm_event_to_string(e0, b);
        printf("Dequeved event\n=====\ntype: %u\n%s\n", e0->type, b);
        sm_queue_to_string(q0, b);
        printf("Queue -1\n=====\n%s\n", b);
        sm_event_free(e0);
    }

    printf("q0 = %p\n", q0);
    sm_queue_to_string(q0, b);
    printf("Purged queue\n=====\n%s\n", b);

    // create synchronized queue q1 with 7 events
    q0 = sm_queue_create(128, true, true, true, true, 7, true);
    sm_queue_to_string(q0, b);
    printf("Queue 7\n=====\n%s\n", b);

    // dequeue all from q0
    while (sm_queue_size(q0) > 0)
    {
        e0 = sm_queue_dequeue(q0);
        sm_event_to_string(e0, b);
        printf("Dequeved event\n=====\ntype: %u\n%s\n", e0->type, b);
        sm_event_free(e0);
    }
    printf("q0.size = %lu\n", sm_queue_size(q0));

    // delete q0
    sm_queue_free(q0);
}