/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Array class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "sm_array.h"

extern __thread void *__sm_tx_desc;

static inline void enqueue(sm_array *a, sm_state *e) 
    __attribute__((always_inline));
static inline sm_state *dequeue(sm_array *q) 
    __attribute__((always_inline));
static inline sm_state *find_by_hash_key(sm_array *a, sm_hash_key *k) 
    __attribute__((always_inline));
static inline sm_state *get_by_hash_key(sm_array *a, sm_hash_key *k)
    __attribute__((always_inline));
static inline sm_state *get_from_queue(sm_array *a)
    __attribute__((always_inline));
static inline void insert_into_hash(sm_array *a, sm_state *s)
    __attribute__((always_inline));
static inline void remove_from_hash(sm_array *a, sm_state *s)
    __attribute__((always_inline));

sm_array *sm_array_create(size_t key_length,
                          size_t queue_size,
                          size_t state_size,
                          struct sm_fsm **fsm,
                          bool synchronized, bool E, bool H, bool K)
{
    sm_array *a;
    int retval;
    if (SM_UNLIKELY((a = aligned_alloc(SM_WORD, sizeof(sm_array))) == NULL))
    {
        SM_REPORT_MESSAGE(SM_LOG_ERR, "aligned_alloc() failed");
        return NULL;
    }
    memset(a, '\0', sizeof(sm_array));
    a->synchronized = synchronized;
    if (a->synchronized)
    {
        pthread_mutexattr_t attr;
        if (SM_UNLIKELY((retval = pthread_mutexattr_init(&attr)) != EXIT_SUCCESS))
        {
            SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_mutexattr_init() failed", retval);
            free(a);
            return NULL;
        }
        if (SM_UNLIKELY((retval = pthread_mutexattr_settype(&attr, SM_MUTEX_TYPE)) != EXIT_SUCCESS))
        {
            SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_mutexattr_settype() failed", retval);
            pthread_mutexattr_destroy(&attr);
            free(a);
            return NULL;
        }
        if (SM_UNLIKELY((retval = pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST)) != EXIT_SUCCESS))
        {
            SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_mutexattr_setrobust() failed", retval);
            pthread_mutexattr_destroy(&attr);
            free(a);
            return NULL;
        }
        if (SM_UNLIKELY((retval = pthread_mutex_init(&(a->lock), &attr)) != EXIT_SUCCESS))
        {
            SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_mutex_init() failed", retval);
            pthread_mutexattr_destroy(&attr);
            free(a);
            return NULL;
        }
        if (SM_UNLIKELY((retval = pthread_mutexattr_destroy(&attr)) != EXIT_SUCCESS))
        {
            SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_mutexattr_destroy() failed", retval);
        }
        if (SM_UNLIKELY((retval = pthread_cond_init(&(a->empty), NULL)) != EXIT_SUCCESS))
        {
            SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_cond_init() failed", retval);
            pthread_mutex_destroy(&(a->lock));
            free(a);
            return NULL;
        }
    }
    a->hash_function = SM_HASH_FUNC;
    a->array_size = hashsize(key_length);
    a->mask = hashmask(key_length);
    sm_state *s;
    if (SM_UNLIKELY((s = sm_state_create(NULL, state_size, a, E, true, H, K)) == NULL))
    {
        SM_REPORT_MESSAGE(SM_LOG_ERR, "sm_state_create() returned NULL");
        sm_array_destroy(&a);
        return NULL;
    }
    a->queue_head = a->queue_tail = s;
    for (size_t i = 0; i < queue_size; i++)
    {
        if (SM_UNLIKELY((s = sm_state_create(fsm, state_size, a, E, true, H, K)) == NULL))
        {
            SM_REPORT_MESSAGE(SM_LOG_ERR, "sm_state_create() returned NULL");
            sm_array_destroy(&a);
            return NULL;
        }
        SM_STATE_DEPOT(s) = a;
        enqueue(a, s);
    }
    if (SM_UNLIKELY((a->table = calloc(a->array_size, sizeof(sm_state *))) == NULL))
    {
        SM_REPORT_MESSAGE(SM_LOG_ERR, "сalloc() failed");
        sm_array_destroy(&a);
        return NULL;
    }
    SM_DEBUG_MESSAGE("sm_array [addr:%p] successfully created", a);
    return a;
}

void sm_array_destroy(sm_array **a)
{
    int retval;
    if ((*a)->synchronized)
    {
        if (SM_UNLIKELY((retval = pthread_mutex_destroy(&((*a)->lock))) != EXIT_SUCCESS))
        {
            SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_mutex_destroy() failed", retval);
        }
        if (SM_UNLIKELY((retval = pthread_cond_destroy(&((*a)->empty))) != EXIT_SUCCESS))
        {
            SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_cond_destroy() failed", retval);
        }
    }
    sm_state *s;
    sm_state *tmp;
    if ((*a)->table != NULL)
    {
        for (size_t i = 0; i < (*a)->array_size; i++)
        {
            s = (*a)->table[i];
            while (s != NULL)
            {
                //s->ctl.D = false;
                tmp = SM_STATE_NEXT(s);
                sm_state_destroy(&s);
                s = tmp;
            }
        }
        free((*a)->table);
    }
    while (SM_STATE_NEXT((*a)->queue_head) != NULL)
    {
        s = dequeue(*a);
        //s->ctl.D = false;
        sm_state_destroy(&s);
    }
    //s = (*a)->queue_head;
    //sm_state_destroy(&s);
    /*
    while(SM_ARRAY_QUEUE_TOP(*a) != NULL)
    {
        s = dequeue(*a);
        sm_state_destroy(&s);
    }
    */
    free(*a);
    SM_DEBUG_MESSAGE("sm_array at [addr:%p] successfully destroyed", *a);
    *a = NULL;
}
/* DEPRECATED
sm_state *sm_array_find_state(sm_array *a, const void *key, size_t key_length)
{
    sm_hash_key k = {NULL, 0, 0xFFFFFFFF};
    sm_hash_set_key(&k, (void *)key, key_length, a->mask);
    int retval;
    if (a->synchronized)
    {
        if (SM_UNLIKELY((retval = pthread_mutex_lock(&(a->lock))) != EXIT_SUCCESS))
        {
            if (retval == EOWNERDEAD)
            {
                SM_SYSLOG(SM_CORE, SM_LOG_WARNING, "pthread owner dead, recovering...", retval);
                retval = pthread_mutex_consistent(&(a->lock));
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
            SM_DEBUG_MESSAGE("mutex [addr:%p] lock successfully acquired", &(a->lock));
        }
    }
    sm_state *s = find_by_hash_key(a, &k);
    if (a->synchronized)
    {
        if (SM_UNLIKELY((retval = pthread_cond_signal(&(a->empty))) != EXIT_SUCCESS))
        {
            SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_cond_signal() failed", retval);
        }
        if (SM_UNLIKELY((retval = pthread_mutex_unlock(&(a->lock))) != EXIT_SUCCESS))
        {
            SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_mutex_unlock() failed", retval);
        }
    }
    SM_DEBUG_MESSAGE("sm_state at [addr:%p] successfully found in the array at [addr:%p]", s, a);
    return s;
}
*/
sm_state *sm_array_get_state(sm_array *a, const void *key, size_t key_length)
{
    sm_hash_key k = {NULL, 0, 0x0};
    sm_hash_set_key(&k, (void *)key, key_length, a->mask);
    int retval;
    if (a->synchronized)
    {
        if (SM_UNLIKELY(retval = pthread_mutex_lock(&(a->lock)) != EXIT_SUCCESS))
        {
            if (retval == EOWNERDEAD)
            {
                SM_SYSLOG(SM_CORE, SM_LOG_WARNING, "pthread owner dead, recovering...", retval);
                retval = pthread_mutex_consistent(&(a->lock));
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
            SM_DEBUG_MESSAGE("mutex [addr:%p] lock successfully acquired", &(a->lock));
        }
    }
    sm_state *s = get_by_hash_key(a, &k);
    if (s == NULL)
    {
        SM_REPORT_MESSAGE(SM_LOG_ERR, "get_by_hash_key() returned NULL");
        return NULL;
    }
    SM_STATE_TX(s) = __sm_tx_desc;
    if (a->synchronized)
    {
        if (SM_UNLIKELY(retval = pthread_cond_signal(&(a->empty)) != EXIT_SUCCESS))
        {
            SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_cond_signal() failed", retval);
        }
        if (SM_UNLIKELY(retval = pthread_mutex_unlock(&(a->lock)) != EXIT_SUCCESS))
        {
            SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_mutex_unlock() failed", retval);
        }
    }
    SM_DEBUG_MESSAGE("state [addr:%p] got successfully from array [addr:%p]", s, a);
    return s;
}

int sm_array_release_state(sm_state **s)
{
    if (SM_UNLIKELY(s == NULL || *s == NULL))
    {
        SM_REPORT_MESSAGE(SM_LOG_WARNING, "an attempt to call release function for the NULL pointer");
        return EXIT_FAILURE;
    }
    if (SM_UNLIKELY(!(*s)->ctl.D))
    {
        SM_REPORT_WARNING("attempt to release a standalone state at [addr:%p] registered in array at [addr:%p]", *s, SM_STATE_DEPOT(*s));
        return EXIT_FAILURE;
    }
    int retval;
    sm_array *a = SM_STATE_DEPOT(*s);
    if (a->synchronized)
    {
        if (SM_UNLIKELY(retval = pthread_mutex_lock(&(a->lock)) != EXIT_SUCCESS))
        {
            if (retval == EOWNERDEAD)
            {
                SM_SYSLOG(SM_CORE, SM_LOG_WARNING, "pthread owner dead, recovering...", retval);
                retval = pthread_mutex_consistent(&(a->lock));
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
            SM_DEBUG_MESSAGE("mutex [addr:%p] lock successfully acquired", &(a->lock));
        }
    }
    if (SM_STATE_TX(*s) != NULL)
    {
        remove_from_hash(a, *s);
        sm_state_erase(*s);
        enqueue(a, *s);
        *s = NULL;
        retval = EXIT_SUCCESS;
    }
    else
    {
        SM_REPORT_WARNING("attempt to release a parked or enqueued state at [addr:%p] registered in array at [addr:%p]", *s, SM_STATE_DEPOT(*s));
        retval = EXIT_FAILURE;
    }
    if (a->synchronized)
    {
        if (SM_UNLIKELY(retval = pthread_cond_signal(&(a->empty)) != EXIT_SUCCESS))
        {
            SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_cond_signal() failed", retval);
        }
        if (SM_UNLIKELY(retval = pthread_mutex_unlock(&(a->lock)) != EXIT_SUCCESS))
        {
            SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_mutex_unlock() failed", retval);
        }
    }
    return retval;
}

int sm_array_park_state(sm_state **s)
{
    if (SM_UNLIKELY(s == NULL || *s == NULL))
    {
        SM_REPORT_MESSAGE(SM_LOG_WARNING, "an attempt to call parking function for the NULL pointer");
        return EXIT_FAILURE;
    }
    if (SM_UNLIKELY(!(*s)->ctl.D))
    {
        SM_REPORT_WARNING("attempt to park a standalone state at [addr:%p] registered in array at [addr:%p]", *s, SM_STATE_DEPOT(*s));
        return EXIT_FAILURE;
    }
    int retval;
    sm_array *a = SM_STATE_DEPOT(*s);
    if (SM_UNLIKELY(!(*s)->ctl.T))
    {
        SM_REPORT_MESSAGE(SM_LOG_WARNING, "state parking function was called for thread-agnostic state");
        return EXIT_FAILURE;
    }
    if (a->synchronized)
    {
        if (SM_UNLIKELY(retval = pthread_mutex_lock(&(a->lock)) != EXIT_SUCCESS))
        {
            if (retval == EOWNERDEAD)
            {
                SM_SYSLOG(SM_CORE, SM_LOG_WARNING, "pthread owner dead, recovering...", retval);
                retval = pthread_mutex_consistent(&(a->lock));
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
            SM_DEBUG_MESSAGE("mutex [addr:%p] lock successfully acquired", &(a->lock));
        }
    }
    SM_STATE_TX(*s) = NULL;
    *s = NULL;
    if (a->synchronized)
    {
        if (SM_UNLIKELY(retval = pthread_cond_signal(&(a->empty)) != EXIT_SUCCESS))
        {
            SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_cond_signal() failed", retval);
        }
        if (SM_UNLIKELY(retval = pthread_mutex_unlock(&(a->lock)) != EXIT_SUCCESS))
        {
            SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_mutex_unlock() failed", retval);
        }
    }
    return EXIT_SUCCESS;
}

int sm_array_to_string(sm_array *a, char *buffer)
{
    char *s = buffer;
    if (SM_UNLIKELY(a == NULL))
    {
        s += sprintf(s, "NULL\n");
    }
    else
    {
        s += sprintf(s, "address: %p\n", a);
        s += sprintf(s, "array size: %u\n", a->array_size);
        s += sprintf(s, "hash mask: %08X\n", a->mask);
        s += sprintf(s, "queue_size: %u\n", a->queue_size);
        s += sprintf(s, "synchronized: %u\n", a->synchronized);
    }
    return (int)((char *)s - (char *)buffer);
}

static inline void enqueue(sm_array *a, sm_state *s)
{
    SM_STATE_NEXT(a->queue_tail) = s;
    SM_STATE_NEXT(s) = NULL;
    a->queue_tail = s;
    a->queue_size++;
}

static inline sm_state *dequeue(sm_array *a)
{
    sm_state *s = SM_STATE_NEXT(a->queue_head);
    if (s != NULL)
    {
        SM_STATE_NEXT(a->queue_head) = SM_STATE_NEXT(s);
        if (SM_STATE_NEXT(a->queue_head) == NULL)
        {
            a->queue_tail = a->queue_head;
        }
        a->queue_size--;
    }
    return s;
}

static inline sm_state *find_by_hash_key(sm_array *a, sm_hash_key *k)
{
    sm_state *s = a->table[k->value];
    while (s != NULL && !sm_hash_key_match(k, SM_STATE_HASH_KEY(s)))
    {
        s = SM_STATE_NEXT(s);
    }
    return s;
}

static inline sm_state *get_by_hash_key(sm_array *a, sm_hash_key *k)
{
    sm_state *s = find_by_hash_key(a, k);
    if (s == NULL)
    {
        if ((s = get_from_queue(a)) == NULL)
        {
            SM_REPORT_MESSAGE(SM_LOG_WARNING, "array queue is unexpectedly empty");
            return NULL;
        }
        //SM_STATE_HASH_KEY(s)->string = SM_STATE_HASH_DST(s); 
        sm_hash_set_key(SM_STATE_HASH_KEY(s), k->string, k->length, a->mask);
        insert_into_hash(a, s);
    }
    return s;
}

static inline sm_state *get_from_queue(sm_array *a)
{
    sm_state *s; 
    int retval;
    if (a->synchronized)
    {
        while ((s = dequeue(a)) == NULL)
        {
            if (SM_UNLIKELY((retval = pthread_cond_wait(&(a->empty), &(a->lock))) != EXIT_SUCCESS))
            {
                SM_SYSLOG(SM_CORE, SM_LOG_ERR, "pthread_cond_wait() failed", retval);
                return NULL;
            }
        }
    }
    else
    {
        s = dequeue(a);
    }
    return s;
}

static inline void insert_into_hash(sm_array *a, sm_state *s)
{
    SM_STATE_NEXT(s) = a->table[SM_STATE_HASH_KEY(s)->value];
    a->table[SM_STATE_HASH_KEY(s)->value] = s;
}

static void remove_from_hash(sm_array *a, sm_state *s)
{
    sm_state **p = &(a->table[SM_STATE_HASH_KEY(s)->value]);
    sm_state *c = a->table[SM_STATE_HASH_KEY(s)->value];
    while(c != NULL && !sm_hash_key_match(SM_STATE_HASH_KEY(s), SM_STATE_HASH_KEY(c)))
    {
        p = &(SM_STATE_NEXT(c));
        c = SM_STATE_NEXT(c);
    }
    if (c != NULL)
    {
        *p = SM_STATE_NEXT(c);
    }
    else
    {
        SM_REPORT_MESSAGE(SM_LOG_INFO, "state object not found in hash table");
    }
}
