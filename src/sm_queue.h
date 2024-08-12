/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Queue class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#ifndef SM_QUEUE_H
#define SM_QUEUE_H

#include "sm_event.h"

typedef struct __attribute__((aligned(SM_WORD))) sm_queue
{
    sm_event *head;
    sm_event *tail;
    size_t size;
    pthread_mutex_t lock;
    pthread_cond_t empty;
    bool synchronized;
} sm_queue;

sm_queue *sm_queue_create(uint32_t event_size,
                          bool K, bool P, bool H,
                          unsigned num_of_events,
                          bool synchronized);
#define SM_QUEUE_CREATE_EMPTY(S) \
    sm_queue_create(0, false, false, false, 0, (S))
void sm_queue_destroy(sm_queue **q);
#define SM_QUEUE_DESTROY(Q) sm_queue_destroy((&(Q)))
#define SM_QUEUE_SIZE(q) (q)->size
#define SM_QUEUE_TOP(q) (q)->head->next
int sm_queue_enqueue(sm_queue *q, sm_event **e);
#define SM_QUEUE_ENQUEUE(Q, E) sm_queue_enqueue((Q), (&(E)))
sm_event *sm_queue_dequeue(sm_queue *q);
int sm_queue_to_string(sm_queue *q, char *buffer);

#endif //SM_QUEUE_H
