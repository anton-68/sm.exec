/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Synchronous queue inner class (to be used by queue, queue2 and array)
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#ifndef SM_SQUEUE_H
#define SM_SQUEUE_H

#include "sm_event.h"

typedef struct __attribute__((aligned(SM_WORD))) sm_squeue
{
    void *head;
    void *tail;
    size_t size;
} sm_squeue;

#define SM_SQUEUE_SIZE(q) (q)->size
#define SM_SQUEUE_TOP(q) (q)->head->next
void sm_squeue_enqueue(sm_squeue *q, sm_event *e) __attribute__((always_inline));
void *sm_squeue_dequeue(sm_squeue *q) __attribute__((always_inline));

#endif //SM_SQUEUE_H
