/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Priority queue class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "sm_pqueue.h"

static inline bool less(sm_pqueue *pq, size_t eh1, size_t eh2)
	__attribute__((always_inline));
static inline void exch(sm_pqueue *pq, size_t e1, size_t e2)
	__attribute__((always_inline));
static inline void swim(sm_pqueue *pq, size_t e)
	__attribute__((always_inline));
static inline void sink(sm_pqueue *pq, size_t e)
	__attribute__((always_inline));
static inline int enqueue(sm_pqueue *pq, sm_event **e)
	__attribute__((always_inline));
static inline sm_event *dequeue(sm_pqueue *pq)
	__attribute__((always_inline));

sm_pqueue *sm_pqueue_create(size_t capacity, bool synchronized)
{
	sm_pqueue *pq;
	int retval;
	if (SM_UNLIKELY((pq = aligned_alloc(SM_WORD, sizeof(sm_pqueue))) == NULL))
	{
		SM_REPORT_MESSAGE(SM_LOG_ERR, "aligned_alloc() failed");
		return NULL;
	}
	pq->capacity = capacity;
	pq->size = 0;
	pq->synchronized = synchronized;
	if (pq->synchronized)
	{
		pthread_mutexattr_t attr;
		if (SM_UNLIKELY((retval = pthread_mutexattr_init(&attr)) != EXIT_SUCCESS))
		{
			SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_mutexattr_init() failed", retval);
			free(pq);
			return NULL;
		}
		if (SM_UNLIKELY((retval = pthread_mutexattr_settype(&attr, SM_MUTEX_TYPE)) != EXIT_SUCCESS))
		{
			SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_mutexattr_settype() failed", retval);
			pthread_mutexattr_destroy(&attr);
			free(pq);
			return NULL;
		}
		if (SM_UNLIKELY((retval = pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST)) != EXIT_SUCCESS))
		{
			SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_mutexattr_setrobust() failed", retval);
			pthread_mutexattr_destroy(&attr);
			free(pq);
			return NULL;
		}
		if (SM_UNLIKELY((retval = pthread_mutex_init(&(pq->lock), &attr)) != EXIT_SUCCESS))
		{
			SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_mutex_init() failed", retval);
			pthread_mutexattr_destroy(&attr);
			free(pq);
			return NULL;
		}
		if (SM_UNLIKELY((retval = pthread_mutexattr_destroy(&attr)) != EXIT_SUCCESS))
		{
			SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_mutexattr_destroy() failed", retval);
		}
		if (SM_UNLIKELY((retval = pthread_cond_init(&(pq->empty), NULL)) != EXIT_SUCCESS))
		{
			SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_cond_init() failed", retval);
			pthread_mutex_destroy(&(pq->lock));
			free(pq);
			return NULL;
		}
	}
	if (SM_UNLIKELY((pq->heap = calloc(capacity, sizeof(sm_event *))) == NULL))
	{
		SM_REPORT_MESSAGE(SM_LOG_ERR, "calloc() failed");
		sm_pqueue_destroy(&pq);
		return NULL;
	}
	SM_DEBUG_MESSAGE("sm_зqueue [addr:%p] successfully created", зq);
	return pq;
}

void sm_pqueue_destroy(sm_pqueue **pq)
{
	int retval;
	for (size_t ep = 0; ep < (*pq)->size; ep++)
	{
		sm_event_destroy(&((*pq)->heap[ep]));
	}
	free((*pq)->heap);
	if ((*pq)->synchronized)
	{

		if (SM_UNLIKELY((retval = pthread_mutex_destroy(&((*pq)->lock))) != EXIT_SUCCESS))
		{
			SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_mutex_destroy() failed", retval);
		}
		if (SM_UNLIKELY((retval = pthread_cond_destroy(&((*pq)->empty))) != EXIT_SUCCESS))
		{
			SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_cond_destroy() failed", retval);
		}
	}
	free(*pq); 
	*pq = NULL;
	SM_DEBUG_MESSAGE("sm_queue [addr:%p] successfully destroyed", *q);
}

int sm_pqueue_enqueue(sm_pqueue *q, sm_event **e)
{
	int retval;
	if (q->synchronized)
	{
		if (SM_UNLIKELY((retval = pthread_mutex_lock(&(q->lock))) != EXIT_SUCCESS))
		{
			if (retval == EOWNERDEAD)
			{
				SM_SYSLOG(SM_CORE, SM_LOG_WARNING, "pthread owner dead, recovering...", retval);
				retval = pthread_mutex_consistent(&(q->lock));
				if (retval == EINVAL || retval == EXIT_SUCCESS)
				{
					SM_SYSLOG(SM_CORE, SM_LOG_NOTICE, "mutex recovered", retval);
				}
				else
				{
					SM_SYSLOG(SM_CORE, SM_LOG_ERR, "mutex recovering failed", retval);
					return retval;
				}
			}
			else
			{
				SM_SYSLOG(SM_CORE, SM_LOG_ERR, "mutex locking failed", retval);
				return retval;
			}
		}
		else
		{
			SM_DEBUG_MESSAGE("mutex [addr:%p] lock successfully acquired", &(q->lock));
		}
	}
	if ((retval = enqueue(q, e)) != EXIT_SUCCESS)
	{
		SM_REPORT_ERROR("pqueue at [addr:%p] is out of capacity, rc = %d", q, retval);
		return retval;
	}
	if (q->synchronized)
	{
		if (SM_UNLIKELY((retval = pthread_cond_signal(&(q->empty))) != EXIT_SUCCESS))
		{
			SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_cond_signal() failed", retval);
		}
		if (SM_UNLIKELY((retval = pthread_mutex_unlock(&(q->lock))) != EXIT_SUCCESS))
		{
			SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_mutex_unlock() failed", retval);
		}
	}
	SM_DEBUG_MESSAGE("event [addr:%p] enqueued successfully into queue [addr:%p]", e, q);
	*e = NULL;
	return EXIT_SUCCESS;
}

sm_event *sm_pqueue_dequeue(sm_pqueue *q)
{
	sm_event *e;
	int retval;
	if (q->synchronized)
	{
		if (SM_UNLIKELY((retval = pthread_mutex_lock(&(q->lock))) != EXIT_SUCCESS))
		{
			if (retval == EOWNERDEAD)
			{
				SM_SYSLOG(SM_CORE, SM_LOG_WARNING, "pthread owner dead, recovering...", retval);
				retval = pthread_mutex_consistent(&(q->lock));
				if (retval == EINVAL || retval == EXIT_SUCCESS)
				{
					SM_SYSLOG(SM_CORE, SM_LOG_NOTICE, "mutex recovered", retval);
				}
				else
				{
					SM_SYSLOG(SM_CORE, SM_LOG_ERR, "mutex recovering failed", retval);
					return NULL;
				}
			}
			else
			{
				SM_SYSLOG(SM_CORE, SM_LOG_ERR, "mutex locking failed", retval);
				return NULL;
			}
		}
		else
		{
			SM_DEBUG_MESSAGE("mutex [addr:%p] lock successfully acquired", &(q->lock));
		}
		while ((e = dequeue(q)) == NULL)
		{
			if (SM_UNLIKELY((retval = pthread_cond_wait(&(q->empty), &(q->lock))) != EXIT_SUCCESS))
			{
				SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_cond_wait() failed", retval);
				return NULL;
			}
		}
		if (SM_UNLIKELY((retval = pthread_mutex_unlock(&(q->lock))) != EXIT_SUCCESS))
		{
			SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_mutex_unlock() failed", retval);
		}
	}
	else
	{
		e = dequeue(q);
	}
	SM_DEBUG_MESSAGE("event [addr:%p] successfully dequeued from pqueue at [addr:%p]", e, q);
	return e;
}

static inline bool less(sm_pqueue *pq, size_t eh1, size_t eh2)
{
	sm_event *e1 = pq->heap[eh1];
	sm_event *e2 = pq->heap[eh2];
	if (SM_EVENT_PRIORITY(e1)[0] < SM_EVENT_PRIORITY(e2)[0])
	{
		return true;
	}
	else
	{
		if (SM_EVENT_PRIORITY(e1)[0] > SM_EVENT_PRIORITY(e2)[0])
		{
			return false;
		}
		else
		{
			if (SM_EVENT_PRIORITY(e1)[1] < SM_EVENT_PRIORITY(e2)[1])
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
}

static inline void exch(sm_pqueue *pq, size_t e1, size_t e2)
{
	sm_event *tmp = pq->heap[e1];
	pq->heap[e1] = pq->heap[e2];
	pq->heap[e2] = tmp;
}

static inline void swim(sm_pqueue *pq, size_t e)
{
	while (e > 0 && less(pq, e / 2, e))
	{
		exch(pq, e, e / 2);
		e = e / 2;
	}
}

static inline void sink(sm_pqueue *pq, size_t e)
{
	while (2 * e < pq->size)
	{
		size_t tmp = 2 * e;
		if (tmp < pq->size - 1 && less(pq, tmp, tmp + 1))
			tmp++;
		if (!less(pq, e, tmp))
			break;
		exch(pq, e, tmp);
		e = tmp;
	}
}

static inline int enqueue(sm_pqueue *pq, sm_event **e)
{
	if (pq->capacity - pq->size <= 0)
		return EXIT_FAILURE;
	pq->heap[pq->size] = *e;
	swim(pq, pq->size++);
	*e = NULL;
	return EXIT_SUCCESS;
}

static inline sm_event *dequeue(sm_pqueue *pq)
{
	if (pq->size <= 0)
		return NULL;
	sm_event *e = pq->heap[0];
	pq->heap[0] = pq->heap[--pq->size];
	pq->heap[pq->size] = NULL;
	sink(pq, 0);
	return e;
}

int sm_pqueue_to_string(sm_pqueue *q, char *buffer)
{
	char *s = buffer;
	if (SM_UNLIKELY(q == NULL))
	{
		s += sprintf(s, "NULL\n");
	}
	else
	{
		s += sprintf(s, "address: %p\n", q);
		s += sprintf(s, "size: %lu\n", q->size);
		s += sprintf(s, "capacity: %lu\n", q->capacity);
		s += sprintf(s, "synchronized: %u\n", q->synchronized);
	}
	return (int)((char *)s - (char *)buffer);
}