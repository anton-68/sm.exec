/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Queue2 class - bi-priority queue
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "sm_queue2.h"

static inline int lock(sm_queue2 *q)
    __attribute__((always_inline));

sm_queue2 *sm_queue2_create()
{
    sm_queue2 *q;
    int retval;
    if ((q = malloc(sizeof(sm_queue2))) == NULL)
    {
        SM_REPORT_MESSAGE(SM_LOG_ERR, "malloc() failed");
        return NULL;
    }
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
    sm_event *e;
    if (SM_UNLIKELY((e = sm_event_create(0, false, false, false, false)) == NULL))
    {
        SM_REPORT_MESSAGE(SM_LOG_ERR, "sm_event_create() returned NULL");
        sm_queue2_destroy(&q);
        return NULL;
    }
    else
    {
        q->h0 = q->t0 = e;
    }
    if (SM_UNLIKELY((e = sm_event_create(0, false, false, false, false)) == NULL))
    {
        SM_REPORT_MESSAGE(SM_LOG_ERR, "sm_event_create() returned NULL");
        sm_queue2_destroy(&q);
        return NULL;
    }
    else
    {
        q->h1 = q->t1 = e;
    }
    return q;
}

void sm_queue2_destroy(sm_queue2 **q2)
{
    int retval;
    sm_queue2 *q = *q2;
    sm_event *tmp;
    sm_event *e = q->h0;
    while (e != NULL)
    {
        tmp = e->next;
        sm_event_destroy(&e);
        e = tmp;
    }
    e = q->h1;
    while (e != NULL)
    {
        tmp = e->next;
        sm_event_destroy(&e);
        e = tmp;
    }
    if (SM_UNLIKELY((retval = pthread_mutex_destroy(&(q->lock))) != EXIT_SUCCESS))
    {
        SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_mutex_destroy() failed", retval);
    }
    if (SM_UNLIKELY((retval = pthread_cond_destroy(&(q->empty))) != EXIT_SUCCESS))
    {
        SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_cond_destroy() failed", retval);
    }
    free(q);
    *q2 = NULL;
}

bool sm_queue2_is_empty(sm_queue2 *q)
{
    return (q->h0->next == NULL && q->h1->next == NULL);
}

void sm_enqueue2(sm_queue2 *q, sm_event **e)
{
    q->t1->next = *e;
    q->t1 = *e;
    q->t1->next = NULL;
    *e = NULL;
}

void sm_enqueue2_high(sm_queue2 *q, sm_event **e)
{
    q->t0->next = *e;
    q->t0 = *e;
    q->t0->next = NULL;
    *e = NULL;
}

int sm_lock_enqueue2(sm_queue2 *q, sm_event **e)
{
    int retval;
    if (SM_UNLIKELY((retval = lock(q)) != EXIT_SUCCESS))
    {
        return retval;
    }
    sm_enqueue2(q, e);
    if (SM_UNLIKELY((retval = pthread_cond_signal(&(q->empty))) != EXIT_SUCCESS))
    {
        SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_cond_signal() failed", retval);
    }
    if (SM_UNLIKELY((retval = pthread_mutex_unlock(&(q->lock))) != EXIT_SUCCESS))
    {
        SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_mutex_unlock() failed", retval);
    }
    SM_DEBUG_MESSAGE("event [addr:%p] enqueued successfully into queue [addr:%p]", *e, q);
    *e = NULL;
    return EXIT_SUCCESS;
}

int sm_lock_enqueue2_high(sm_queue2 *q, sm_event **e)
{
    int retval;
    if (SM_UNLIKELY((retval = lock(q)) != EXIT_SUCCESS))
    {
        return retval;
    }
    sm_enqueue2_high(q, e);
    if (SM_UNLIKELY((retval = pthread_cond_signal(&(q->empty))) != EXIT_SUCCESS))
    {
        SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_cond_signal() failed", retval);
    }
    if (SM_UNLIKELY((retval = pthread_mutex_unlock(&(q->lock))) != EXIT_SUCCESS))
    {
        SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_mutex_unlock() failed", retval);
    }
    SM_DEBUG_MESSAGE("event [addr:%p] enqueued successfully into queue [addr:%p]", *e, q);
    *e = NULL;
    return EXIT_SUCCESS;
}

static sm_event *sm_dequeue2_low(sm_queue2 *q)
{
    sm_event *e = q->h1->next;
    if (e != NULL)
    {
        q->h1->next = q->h1->next->next;
        if (e->next == NULL)
            q->t1 = q->h1;
        else
            e->next = NULL;
    }
    return e;
}

static sm_event *sm_dequeue2_high(sm_queue2 *q)
{
    sm_event *e = q->h0->next;
    if (e != NULL)
    {
        q->h0->next = q->h0->next->next;
        if (e->next == NULL)
            q->t0 = q->h0;
        else
            e->next = NULL;
    }
    return e;
}

sm_event *sm_dequeue2(sm_queue2 *q)
{
    sm_event *e;
    if (q->h0->next != NULL)
        e = sm_dequeue2_high(q);
    else
        e = sm_dequeue2_low(q);
    return e;
}

sm_event *sm_lock_dequeue2(sm_queue2 *q)
{
    sm_event *e;
    int retval = EXIT_SUCCESS;
    if (SM_UNLIKELY((retval = lock(q)) != EXIT_SUCCESS))
    {
        SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_mutex_unlock()", retval);
        return NULL;
    }
    e = sm_dequeue2(q);
    if (e != NULL)
    {
        if (SM_UNLIKELY((retval = pthread_mutex_unlock(&(q->lock))) != EXIT_SUCCESS))
        {
            SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_mutex_unlock() failed", retval);
        }
    }
    else
    {
        while ((e = sm_dequeue2(q)) == NULL)
        {
            if (pthread_cond_wait(&(q->empty), &(q->lock)) != EXIT_SUCCESS)
            {
                SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_cond_wait()", retval);
                return NULL;
            }
        }
        if (SM_UNLIKELY((retval = pthread_mutex_unlock(&(q->lock))) != EXIT_SUCCESS))
        {
            SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_mutex_unlock() failed", retval);
        }
        SM_DEBUG_MESSAGE("event [addr:%p] dequeued successfully into queue [addr:%p]", e, q);
    }
    return e;
}

int sm_queue2_to_string(sm_queue2 *q, char *buffer)
{
    char *s = buffer;
    if (SM_UNLIKELY(q == NULL))
    {
        s += sprintf(s, "NULL\n");
    }
    else
    {
        s += sprintf(s, "address: %p\n", q);
        s += sprintf(s, "top low event:\n");
        s += sm_event_to_string(SM_QUEUE2_TOP_LOW(q), s);
        s += sprintf(s, "\ntop high event:\n");
        s += sm_event_to_string(SM_QUEUE2_TOP_HIGH(q), s);
    }
    return (int)((char *)s - (char *)buffer);
}

static inline int lock(sm_queue2 *q)
{
    int retval = EXIT_SUCCESS;
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
    return retval;
}
