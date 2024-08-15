/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Priority queue class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#ifndef SM_PQUEUE_H
#define SM_PQUEUE_H

#include "sm_event.h"                                            

typedef struct sm_pqueue {
	size_t capacity;
	size_t size;
	pthread_mutex_t lock;
	pthread_cond_t empty;
	sm_event **heap;
	bool synchronized;
} sm_pqueue;

sm_pqueue *sm_pqueue_create(size_t capacity, bool synchronized);
void sm_pqueue_destroy(sm_pqueue **q);
#define SM_PQUEUE_DESTROY(Q) sm_queue_destroy((&(Q)))
#define SM_PQUEUE_SIZE(q) (q)->size
#define SM_PQUEUE_TOP(q) (q)->size == 0 ? NULL : (q)->heap[0]
int sm_pqueue_enqueue(sm_pqueue *q, sm_event **e);
#define SM_PQUEUE_ENQUEUE(Q, E) sm_pqueue_enqueue((Q), (&(E)))
sm_event *sm_pqueue_dequeue(sm_pqueue *q);
int sm_pqueue_to_string(sm_pqueue *q, char *buffer);

#endif //SM_PQUEUE_H