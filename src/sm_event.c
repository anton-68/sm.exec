/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Event class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "sm_event.h"

static inline size_t event_sizeof(const sm_event *e) 
                     __attribute__((always_inline));
static inline sm_event *event_chain_end(sm_event *e)
                        __attribute__((always_inline));

size_t sm_event_sizeof(const sm_event *e)
{
    return event_sizeof(e);
}

sm_event *sm_event_create(uint32_t size, bool Q, bool K, bool P, bool H)
{
    sm_event header;
    header.ctl.size = size >> 6;
    header.ctl.Q = Q;
    header.ctl.K = K;
    header.ctl.P = P;
    header.ctl.H = H;
    header.ctl.L = false;
    header.ctl.D = true;
    sm_event *e;
    if (SM_UNLIKELY((e = aligned_alloc(SM_WORD, sm_event_sizeof(&header))) == NULL))
    {
        SM_REPORT_MESSAGE(SM_LOG_ERR, "malloc() failed");
        return NULL;
    }
    memset(e, '\0', sm_event_sizeof(&header));
    e->type = header.type;
    SM_DEBUG_MESSAGE("sm_event [addr:%p] successfully created", e);
    return e;
}

void sm_event_destroy(sm_event **e)
{
    if (e != NULL && *e != NULL)
    {
        free(*e);
    }
    SM_DEBUG_MESSAGE("sm_event [addr:%p] successfully destroyed", e);
    *e = NULL;
} 

void sm_event_erase(sm_event *e)
{
    if (e != NULL)
    {
        uint32_t type = e->type;
        struct sm_queue *q = SM_EVENT_DEPOT(e);
        memset(e, 0, sm_event_sizeof(e));
        e->type = type;
        SM_EVENT_DEPOT(e) = q;
    }
    SM_DEBUG_MESSAGE("sm_event [addr:%p] successfully erased", e);
}

int sm_queue_enqueue(struct sm_queue *q, sm_event **e);
void sm_event_dispose(sm_event **e)
{
    sm_event *next_e;
    while (*e != NULL)
    {
        if ((*e)->ctl.L)
        {
            next_e = (*e)->next;
        }
        else
        {
            next_e = NULL;
        }
        if ((*e)->ctl.Q)
        {
            sm_event_erase(*e);
            sm_queue_enqueue(SM_EVENT_DEPOT(*e), e);
        }
        else
        {
            sm_event_destroy(e);
        }
        *e = next_e;
    };
}

sm_event *sm_queue_dequeue(struct sm_queue *q);
sm_event *sm_event_clone(sm_event *e)
{
    sm_event *c;
    if (e != NULL)
    {
        size_t size_e = sm_event_sizeof(e);
        if (e->ctl.Q)
        {
            c = sm_queue_dequeue(SM_EVENT_DEPOT(e));
        }
        else
        {
            c = aligned_alloc(SM_WORD, size_e);
            if (SM_UNLIKELY(c == NULL))
            {
                SM_REPORT_MESSAGE(SM_LOG_ERR, "aligned_alloc() failed");
                return NULL;
            }
        }
        memcpy(c, e, size_e);
        SM_DEBUG_MESSAGE("event clone [addr:%p] for sm_event [addr:%p] successfully created", c, e);
    }
    else
    {
        SM_DEBUG_MESSAGE("event cloning for sm_event [addr:NULL] failed");
        c = NULL;
    }
    return c;
}

sm_event *sm_event_chain_end(sm_event *e)
{
    sm_event *e1 = event_chain_end(e);
    SM_DEBUG_MESSAGE("end of chain event [addr:%p] for sm_event [addr:%p] successfully found", e1, e);
    return e1;
}

void sm_event_chain(sm_event *e0, sm_event **e1)
{   
    if (SM_UNLIKELY(e0 == NULL || *e1 == NULL))
    {
        SM_SYSLOG(SM_CORE, SM_LOG_ERR, "NULL event sent to function", EXIT_FAILURE);
        return;
    }
    sm_event *e2 = sm_event_chain_end(e0);
    e2->ctl.L = true;
    e2->next = *e1;
    SM_DEBUG_MESSAGE("events [addr:%p] and [addr:%p] successfully chained", e0, e1);
    *e1 = NULL;
}

void sm_event_unchain(sm_event *e)
{
    if (SM_UNLIKELY(e == NULL))
    {
        SM_SYSLOG(SM_CORE, SM_LOG_ERR, "NULL event sent to function", EXIT_FAILURE);
        return;
    }
    e->ctl.L = false;
    e->next = NULL;
    SM_DEBUG_MESSAGE("event [addr:%p] successfully unchained", e);
}

int sm_event_to_string(const sm_event *e, char *buffer) {
    char *s = buffer;
    if (SM_UNLIKELY(e == NULL))
    {
        s += sprintf(s, "NULL\n");
    }
    else
    {
        char data_buffer[32];
        s += sprintf(s, "address: %p\n", e);
        s += sprintf(s, "next: %p\n", e->next);
        s += sprintf(s, "service_id: %u\n", e->service_id);
        s += sprintf(s, "event_id: %u\n", e->event_id);
        s += sprintf(s, "size: %u B\n", e->ctl.size << 6);
        if(e->ctl.Q)
            s += sprintf(s, "depot addr: %p\n", SM_EVENT_DEPOT(e));
        if (e->ctl.K)
        {
            /* s += sprintf(s, "key string: %s\n", SM_EVENT_KEY_STRING(e)  != NULL ?
                                                       SM_EVENT_KEY_STRING(e) : "\0");
               s += sprintf(s, "key length: %d\n", SM_EVENT_KEY_LENGTH(e));
               s += sprintf(s, "key hash: 0x%X\n", SM_EVENT_KEY_HASH(e)); */
            s += sprintf(s, "hash key addr: %p\n", SM_EVENT_HASH_KEY(e));
            s += sprintf(s, "key string: %s\n", (char *)SM_EVENT_HASH_KEY(e)->string);
            s += sprintf(s, "key length: %d\n", SM_EVENT_HASH_KEY(e)->length);
            s += sprintf(s, "key hash: 0x%X\n", SM_EVENT_HASH_KEY(e)->value);
        }
        if (e->ctl.P) 
        {
            s += sprintf(s, "priority[0]: %lu\n", SM_EVENT_PRIORITY(e)[0]);
            s += sprintf(s, "priority[1]: %lu\n", SM_EVENT_PRIORITY(e)[1]);
        }
        if (e->ctl.H)
            s += sprintf(s, "handle: %p\n", SM_EVENT_HANDLE(e));
        s += sprintf(s, "linked: %s\n", e->ctl.L ? "true" : "false");
        s += sprintf(s, "disposable: %s\n", e->ctl.D ? "true" : "false");
        
        if (e->ctl.size > 0) {
            snprintf(data_buffer, 32, "%s", (char *)SM_EVENT_DATA(e));
            s += sprintf(s, "data:\n[%s ...]\n", data_buffer);
        }
    }
    return (int)((char *)s - (char *)buffer);
}

static inline size_t event_sizeof(const sm_event *e)
{
    return sizeof(sm_event) + 
           sizeof(sm_hash_key) * !!e->ctl.K + 
           SM_WORD * (!!e->ctl.Q + 2 * !!e->ctl.P + !!e->ctl.H) + 
           SM_EVENT_DATA_SIZE(e);
}

static inline  sm_event *event_chain_end(sm_event *e)
{
    sm_event *e1 = e;
    while (e1 != NULL && e1->ctl.L)
    {
        e1 = e1->next;
    }
    return e1;
}