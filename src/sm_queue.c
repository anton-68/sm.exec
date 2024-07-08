/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Queue class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "sm_queue.h"

// Private methods

static inline void enqueue(sm_event *e, sm_queue *q) 
						   __attribute__((always_inline));
static inline sm_event *dequeue(sm_queue *q) 
								__attribute__((always_inline));


// Public methods

sm_queue *sm_queue_create(uint32_t event_size,
						  bool Q, bool K, bool P, bool H,
						  unsigned num_of_events,
						  bool synchronized)
{
	int retval;
	char message[80];
	sm_queue *q;
	if (SM_UNLIKELY((q = malloc(sizeof(sm_queue))) == NULL))
	{
		SM_REPORT(SM_LOG_ERR, "malloc() returned NULL");
		return NULL;
	}
	sm_event *e;
	if (SM_UNLIKELY((e = sm_event_create(0, false, false, false, false)) == NULL))
	{
		SM_REPORT(SM_LOG_ERR, "sm_event_create() returned NULL");
		free(q);
		return NULL;
	}
	q->head = q->tail = e;
	q->size = 0;
	for (int i = 0; i < num_of_events; i++)
	{
		if (SM_UNLIKELY((e = sm_event_create(event_size, Q, K, P, H)) == NULL))
		{
			SM_REPORT(SM_LOG_ERR, "event_create()");
			sm_queue_free(q);
			return NULL;
		}
		SM_EVENT_DEPOT(e) = q;
		enqueue(e, q);
	}
	q->synchronized = synchronized;
	if (q->synchronized)
	{
		pthread_mutexattr_t attr;
		if (SM_UNLIKELY(retval = pthread_mutexattr_init(&attr) != EXIT_SUCCESS))
		{
			sprintf(message, "pthread_mutexattr_init() failed with code %i", retval);
			SM_REPORT(SM_LOG_ERR, message);
			sm_queue_free(q);
			return NULL;
		}
		if (SM_UNLIKELY(retval = pthread_mutexattr_settype(&attr, SM_MUTEX_TYPE) != EXIT_SUCCESS))
		{
			sprintf(message, "pthread_mutexattr_settype() failed with code %i", retval);
			SM_REPORT(SM_LOG_ERR, message);
			sm_queue_free(q);
			return NULL;
		}
		if (SM_UNLIKELY(retval = pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST) != EXIT_SUCCESS))
		{
			sprintf(message, "pthread_mutexattr_setrobust() failed with code %i", retval);
			SM_REPORT(SM_LOG_ERR, message);
			sm_queue_free(q);
			return NULL;
		}
		if (SM_UNLIKELY(retval = pthread_mutex_init(&(q->lock), &attr) != EXIT_SUCCESS))
		{
			sprintf(message, "pthread_mutex_init() failed with code %i", retval);
			SM_REPORT(SM_LOG_ERR, message);
			sm_queue_free(q);
			return NULL;
		}
		if (SM_UNLIKELY(retval = pthread_mutexattr_destroy(&attr) != EXIT_SUCCESS))
		{
			sprintf(message, "pthread_mutexattr_destroy() failed with code %i", retval);
			SM_REPORT(SM_LOG_ERR, message);
		}
		if (SM_UNLIKELY(retval = pthread_cond_init(&(q->empty), NULL) != EXIT_SUCCESS))
		{
			sprintf(message, "pthread_cond_init() failed with code %i", retval);
			SM_REPORT(SM_LOG_ERR, message);
			sm_queue_free(q);
			return NULL;
		}
	}
	SM_REPORT_MESSAGE(SM_LOG_DEBUG, "sm_queue successfully created");
	return q;
}

void sm_queue_free(sm_queue *q)
{
	int retval;
	char message[80];
	sm_event *tmp;
	// lock ??
	sm_event *e = q->head;
	while (e != NULL)
	{
		tmp = e->next;
		sm_event_free(e);
		e = tmp;
	}
	// unlock ??
	if (q->synchronized)
	{
		if (SM_UNLIKELY(retval = pthread_mutex_destroy(&q->lock) != EXIT_SUCCESS))
		{
			sprintf(message, "pthread_mutex_destroy() failed with code %i", retval);
			SM_REPORT(SM_LOG_ERR, message);
		}
		if (SM_UNLIKELY(retval = pthread_cond_destroy(&q->empty) != EXIT_SUCCESS))
		{
			sprintf(message, "pthread_cond_destroy() failed with code %i", retval);
			SM_REPORT(SM_LOG_ERR, message);
		}
		free(q);
	}
	SM_REPORT_MESSAGE(SM_LOG_DEBUG, "sm_queue destroyed");
}

int sm_queue_enqueue(sm_event *e, sm_queue *q)
{
	int retval;
	if (q->synchronized)
	{
		if (SM_UNLIKELY(retval = pthread_mutex_lock(&(q->lock)) != EXIT_SUCCESS))
		{
			if (retval == EOWNERDEAD)
			{
				SM_REPORT_CODE(SM_LOG_WARNING, retval);
				retval = pthread_mutex_consistent(&(q->lock));
				if (retval == EINVAL || retval == EXIT_SUCCESS)
				{
					SM_REPORT_CODE(SM_LOG_NOTICE, retval);
				}
				else
				{
					SM_REPORT_CODE(SM_LOG_ERR, retval);
					return retval;
				}
			}
			else
			{
				SM_REPORT_CODE(SM_LOG_ERR, retval);
				return retval;
			}
		}
	}
	enqueue(e, q);
	if (q->synchronized)
	{
		if (SM_UNLIKELY(retval = pthread_cond_signal(&(q->empty)) != EXIT_SUCCESS))
		{
			SM_REPORT_CODE(SM_LOG_ERR, retval);
			return retval;
		}
		if (SM_UNLIKELY(retval = pthread_mutex_unlock(&(q->lock)) != EXIT_SUCCESS))
		{
			SM_REPORT_CODE(SM_LOG_ERR, retval);
			return retval;
		}
	}
	SM_REPORT_MESSAGE(SM_LOG_DEBUG, "event enqueed");
	return EXIT_SUCCESS;
}

sm_event *sm_queue_dequeue(sm_queue *q)
{
	sm_event *e;
	int retval;
	if (q->synchronized)
	{
		if (SM_UNLIKELY(retval = pthread_mutex_lock(&(q->lock)) != EXIT_SUCCESS))
		{
			if(retval == EOWNERDEAD)
			{
				SM_REPORT_CODE(SM_LOG_WARNING, retval);
				retval = pthread_mutex_consistent(&(q->lock));
				if (retval == EINVAL || retval == EXIT_SUCCESS)
				{
					SM_REPORT_CODE(SM_LOG_NOTICE, retval);
				}
				else {
					SM_REPORT_CODE(SM_LOG_ERR, retval);
				}
			}
			else {
				SM_REPORT_CODE(SM_LOG_ERR, retval);
				return NULL;
			}
		}
		while ((e = dequeue(q)) == NULL)
		{
			if (SM_UNLIKELY(retval = pthread_cond_wait(&(q->empty), &(q->lock)) != EXIT_SUCCESS))
			{
				SM_REPORT_CODE(SM_LOG_ERR, retval);
				return NULL;
			}
		}
		if (SM_UNLIKELY(retval = pthread_mutex_unlock(&(q->lock)) != EXIT_SUCCESS))
		{
			SM_REPORT_CODE(SM_LOG_ERR, retval);
		}
	}
	else
		e = dequeue(q);
	SM_REPORT_MESSAGE(SM_LOG_DEBUG, "event successfully dequeued");
	return e;
}

int sm_queue_to_string(sm_queue *q, char *buffer)
{
	char *s = buffer;
	s += sprintf(s, "address: %p\n", q);
	s += sprintf(s, "size: %lu\n", q->size);
	s += sprintf(s, "synchronized: %u\n", q->synchronized);
	return (int)((char *)s - (char *)buffer);
}

// Private methods

static inline void enqueue(sm_event *e, sm_queue *q)
{
	q->tail->next = e;
	q->tail = e;
	q->tail->next = NULL;
	q->size++;
}

static inline sm_event *dequeue(sm_queue *q)
{
	sm_event *e = q->head->next;
	if (e != NULL)
	{
		q->head->next = q->head->next->next;
		if (e->next == NULL)
			q->tail = q->head;
		else
			e->next = NULL;
		q->size--;
	}
	return e;
}
