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

sm_event *sm_queue2_get(const sm_queue2 *q);
sm_event *sm_queue2_get_high(const sm_queue2 *q);
void sm_enqueue2(sm_event **e, sm_queue2 *q);
#define SM_QUEUE2_ENQUEUE(E, Q) sm_enqueue2((&(E)), (Q))
void sm_enqueue2_high(sm_event **e, sm_queue2 *q);
#define SM_QUEUE2_ENQUEUE_HIGH(E, Q) sm_enqueue2_high((&(E)), (Q))
int sm_lock_enqueue2(sm_event **e, sm_queue2 *q);
#define SM_QUEUE2_LOCK_ENQUEUE(E, Q) sm_lock_enqueue2((&(E)), (Q))
int sm_lock_enqueue2_high(sm_event **e, sm_queue2 *q);
#define SM_QUEUE2_LOCK_ENQUEUE_HIGH(E, Q) sm_lock_enqueue2_high((&(E)), (Q))
sm_event *sm_dequeue2(sm_queue2 *q);
sm_event *sm_lock_dequeue2(sm_queue2 *q);
int sm_queue2_to_string(sm_queue2 *q, char *buffer);

#endif //SM_QUEUE2_H
