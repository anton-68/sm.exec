/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Synchronous queue inner class (to be used by queue, queue2 and array)
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "sm_squeue.h"

inline void sm_squeue_enqueue(sm_squeue *q, sm_event *e)
{
	q->tail->next = e;
	q->tail = e;
	q->tail = sm_event_chain_end(e);
	q->tail->next = NULL;
	q->size++;
}

inline void *sm_squeue_dequeue(sm_squeue *q)
{
	void *e = q->head->next;
	void *end = sm_event_chain_end(e);
	q->head->next = end->next;
	if (q->head->next == NULL)
	{
		q->tail = q->head;
	}
	q->size--;
	end->next = NULL;
	return e;
}
