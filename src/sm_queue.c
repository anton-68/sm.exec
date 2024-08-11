/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Queue class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "sm_queue.h"

static inline void enqueue(sm_queue *q, sm_event *e) 
	__attribute__((always_inline));
static inline sm_event *dequeue(sm_queue *q) 
	__attribute__((always_inline));

sm_queue *sm_queue_create(uint32_t event_size,
						  bool K, bool P, bool H,
						  unsigned num_of_events,
						  bool synchronized)
{
	sm_queue *q;
	int retval;
	if (SM_UNLIKELY((q = aligned_alloc(SM_WORD, sizeof(sm_queue))) == NULL))
	{
		SM_REPORT_MESSAGE(SM_LOG_ERR, "aligned_alloc() failed");
		return NULL;
	}
	q->synchronized = synchronized;
	if (q->synchronized)
	{
		pthread_mutexattr_t attr;
		if (SM_UNLIKELY((retval = pthread_mutexattr_init(&attr)) != EXIT_SUCCESS))
		{
			SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_mutexattr_init() failed", retval);
			free(q);
			return NULL;
		}
		if (SM_UNLIKELY((retval = pthread_mutexattr_settype(&attr, SM_MUTEX_TYPE)) != EXIT_SUCCESS))
		{
			SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_mutexattr_settype() failed", retval);
			pthread_mutexattr_destroy(&attr);
			free(q);
			return NULL;
		}
		if (SM_UNLIKELY((retval = pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST)) != EXIT_SUCCESS))
		{
			SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_mutexattr_setrobust() failed", retval);
			pthread_mutexattr_destroy(&attr);
			free(q);
			return NULL;
		}
		if (SM_UNLIKELY((retval = pthread_mutex_init(&(q->lock), &attr)) != EXIT_SUCCESS))
		{
			SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_mutex_init() failed", retval);
			pthread_mutexattr_destroy(&attr);
			free(q);
			return NULL;
		}
		if (SM_UNLIKELY((retval = pthread_mutexattr_destroy(&attr)) != EXIT_SUCCESS))
		{
			SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_mutexattr_destroy() failed", retval);
		}
		if (SM_UNLIKELY((retval = pthread_cond_init(&(q->empty), NULL)) != EXIT_SUCCESS))
		{
			SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_cond_init() failed", retval);
			pthread_mutex_destroy(&(q->lock));
			free(q);
			return NULL;
		}
	}
	sm_event *e;
	if (SM_UNLIKELY((e = sm_event_create(0, false, false, false, false)) == NULL))
	{
		SM_REPORT_MESSAGE(SM_LOG_ERR, "sm_event_create() returned NULL");
		sm_queue_destroy(&q);
		return NULL;
	}
	q->head = q->tail = e;
	q->size = 0;
	for (int i = 0; i < num_of_events; i++)
	{
		if (SM_UNLIKELY((e = sm_event_create(event_size, true, K, P, H)) == NULL))
		{
			SM_REPORT_MESSAGE(SM_LOG_ERR, "sm_event_create() returned NULL");
			sm_queue_destroy(&q);
			return NULL;
		}
		SM_EVENT_DEPOT(e) = q;
		enqueue(q, e);
	}

	SM_DEBUG_MESSAGE("sm_queue [addr:%p] successfully created", q);
	return q;
}

void sm_queue_destroy(sm_queue **q)
{
	int retval;
	if ((*q)->synchronized)
	{
		if (SM_UNLIKELY((retval = pthread_mutex_destroy(&((*q)->lock))) != EXIT_SUCCESS))
		{
			SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_mutex_destroy() failed", retval);
		}
		if (SM_UNLIKELY((retval = pthread_cond_destroy(&((*q)->empty))) != EXIT_SUCCESS))
		{
			SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_cond_destroy() failed", retval);
		}
	}
	sm_event *tmp;
	sm_event *e = (*q)->head;
	while (e != NULL)
	{
		tmp = e->next;
		sm_event_destroy(&e);
		e = tmp;
	}
	free(*q);
	SM_DEBUG_MESSAGE("sm_queue [addr:%p] successfully destroyed", *q);
	*q = NULL;
}

int sm_queue_enqueue(sm_queue *q, sm_event **e)
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
	enqueue(q, *e);
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

sm_event *sm_queue_dequeue(sm_queue *q)
{
	sm_event *e;
	int retval;
	if (q->synchronized)
	{
		if (SM_UNLIKELY((retval = pthread_mutex_lock(&(q->lock))) != EXIT_SUCCESS))
		{
			if(retval == EOWNERDEAD)
			{
				SM_SYSLOG(SM_CORE, SM_LOG_WARNING, "pthread owner dead, recovering...", retval);
				retval = pthread_mutex_consistent(&(q->lock));
				if (retval == EINVAL || retval == EXIT_SUCCESS)
				{
					SM_SYSLOG(SM_CORE, SM_LOG_NOTICE, "mutex recovered", retval);
				}
				else {
					SM_SYSLOG(SM_CORE, SM_LOG_ERR, "mutex recovering failed", retval);
					return NULL;
				}
			}
			else {
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
	SM_DEBUG_MESSAGE("event [addr:%p] successfully dequeued from queue [addr:%p]", e, q);
	return e;
}

int sm_queue_to_string(sm_queue *q, char *buffer)
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
		s += sprintf(s, "synchronized: %u\n", q->synchronized);
	}
	return (int)((char *)s - (char *)buffer);
}

static inline void enqueue(sm_queue *q, sm_event *e)
{
	q->tail->next = e;
	q->tail = e;
	q->tail = sm_event_chain_end(e);
/*	while (e->ctl.L && e->next != NULL)
	{
		q->tail = e->next;
	} */
	q->tail->next = NULL;
	q->size++;
}

static inline sm_event *dequeue(sm_queue *q)
{
	sm_event *e = q->head->next;
	sm_event *end = sm_event_chain_end(e);
	q->head->next = end->next;
	if (q->head->next == NULL)
	{
		q->tail = q->head;
	}
	q->size--;
	end->next = NULL;
	return e;
}
