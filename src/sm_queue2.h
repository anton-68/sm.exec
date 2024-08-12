/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Queue2 class - bi-priority queue
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#ifndef SM_QUEUE2_H
#define SM_QUEUE2_H

#include "sm_event.h"

typedef struct sm_queue2 
{
    pthread_mutex_t lock;
    pthread_cond_t empty;
    sm_event * h0;
    sm_event * t0;
    sm_event * h1;
    sm_event * t1;
} sm_queue2;

sm_queue2 *sm_queue2_create();
void sm_queue2_destroy(sm_queue2 **q2);
#define SM_QUEUE2_DESTROY(Q) sm_queue2_destroy((&(Q)))
bool sm_queue2_is_empty(sm_queue2 *q);
#define SM_QUEUE2_TOP_LOW(q) (q)->h1->next
#define SM_QUEUE2_TOP_HIGH(q) (q)->h0->next
void sm_enqueue2(sm_queue2 *q, sm_event **e);
#define SM_QUEUE2_ENQUEUE(Q, E) sm_enqueue2((Q), (&(E)))
void sm_enqueue2_high(sm_queue2 *q, sm_event **e);
#define SM_QUEUE2_ENQUEUE_HIGH(Q, E) sm_enqueue2_high((Q), (&(E)))
int sm_lock_enqueue2(sm_queue2 *q, sm_event **e);
#define SM_QUEUE2_LOCK_ENQUEUE(Q, E) sm_lock_enqueue2((Q), (&(E)))
int sm_lock_enqueue2_high(sm_queue2 *q, sm_event **e);
#define SM_QUEUE2_LOCK_ENQUEUE_HIGH(Q, E) sm_lock_enqueue2_high((Q), (&(E)))

sm_event *sm_dequeue2(sm_queue2 *q);
sm_event *sm_lock_dequeue2(sm_queue2 *q);
int sm_queue2_to_string(sm_queue2 *q, char *buffer);

#endif //SM_QUEUE2_H
