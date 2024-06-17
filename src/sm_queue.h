/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Queue class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#ifndef SM_QUEUE_H
#define SM_QUEUE_H

#include "sm_event.h"

typedef struct sm_queue {
    pthread_mutex_t lock;
    pthread_cond_t empty;
    sm_event * head;
    sm_event * tail;
	bool synchronized;
	size_t size;
} sm_queue;

// Public methods

#define SM_QUEUE_CREATE_EMPTY(S) \
    sm_queue_create(0, false, false, false, false, 0, (S))

#define SM_QUEUE_SIZE(q) (q)->size

sm_queue *sm_queue_create(uint32_t event_size,
                          bool Q, bool K, bool P, bool H,
                          unsigned num_of_events,
                          bool synchronized);
void sm_queue_free(sm_queue *q);
size_t sm_queue_size(sm_queue *q);

sm_event * sm_queue_top(const sm_queue * q);
int sm_queue_enqueue(sm_event *e, sm_queue *q);
sm_event *sm_queue_dequeue(sm_queue *q);

int sm_queue_to_string(sm_queue *q, char *buffer);

#endif //SM_QUEUE_H
