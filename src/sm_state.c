/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
State class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "sm_state.h"

extern __thread void *__sm_tx_desc;

static inline size_t sm_state_sizeof(sm_state *s) __attribute__((always_inline));

sm_state *sm_state_create(sm_fsm **f, uint32_t size, bool D, bool E, bool T, bool H, bool K)
{
    if(SM_UNLIKELY(f == NULL)){
        SM_REPORT_MESSAGE(SM_LOG_INFO, "NULL FSM pointer on input");
        //return NULL;
    }
    sm_state header;
    header.ctl.size = size >> 6;
    header.ctl.D = D;
    header.ctl.E = E;
    header.ctl.T = T;
    header.ctl.H = H;
    header.ctl.K = K;
    sm_state *s;
    if (SM_UNLIKELY((s = aligned_alloc(SM_WORD, sm_state_sizeof(&header))) == NULL))
    {
        SM_REPORT_MESSAGE(SM_LOG_ERR, "malloc() failed");
        return NULL;
    }
    memset(s, '\0', sm_state_sizeof(&header));
    s->type = header.type;
    s->fsm = f; 
    if (f != NULL && *f != NULL)
    {
        s->state_id = (*f)->initial;
    }
    if (s->ctl.T)
    {
        SM_STATE_TX(s) = __sm_tx_desc; 
    }
    if (s->ctl.D && s->ctl.K)
    {
        SM_STATE_HASH_KEY(s)->string = SM_STATE_DATA(s); // because SM_STATE_HASH_KEY(s)->string == 0 here
    }
    SM_DEBUG_MESSAGE("sm_state [addr:%p] successfully created with size = %lu", s, sm_state_sizeof(&header));
    return s;
}

void sm_state_destroy(sm_state **s)
{
    if (*s != NULL) 
    {
        free(*s);
    }
    SM_DEBUG_MESSAGE("sm_state at [addr:%p] successfully destroyed", *s);
    *s = NULL;
}

void sm_state_erase(sm_state *s) {
    if(s != NULL)
    {
        uint32_t type = s->type;
        struct sm_array *a = SM_STATE_DEPOT(s);
        memset(s, '\0', sm_state_sizeof(s));
        s->type = type;
        if (s->ctl.D && s->ctl.K)
        {
            SM_STATE_HASH_KEY(s)->string = SM_STATE_DATA(s); // because SM_STATE_HASH_KEY(s)->string == 0 here
        }
        SM_STATE_DEPOT(s) = a;
    }
    SM_DEBUG_MESSAGE("sm_state [addr:%p] successfully erased", s);
}

void sm_array_park_state(struct sm_array *, sm_state *);
void sm_state_dispose(sm_state **s) {
    if ((*s)->ctl.E)
    {
        sm_event_dispose(&(SM_STATE_EVENT_TRACE(*s)));
    }
    if ((*s)->ctl.D)
    {
        sm_state_erase(*s);
        sm_array_park_state(SM_STATE_DEPOT(s), s);
    }
    else
    {
        sm_state_destroy(s);
    }
    SM_DEBUG_MESSAGE("sm_state [addr:%p] successfully parked", s);
}

void sm_state_push_event(sm_state *s, sm_event **e)
{
    if (s->ctl.E)
    {
        (*e)->next = SM_STATE_EVENT_TRACE(s);
        SM_STATE_EVENT_TRACE(s) = *e;
        (*e)->ctl.L = true;
    }
    *e = NULL;
    SM_DEBUG_MESSAGE("sm_event [addr:%p] successfully pushed into stack of sm_state at [addr:%p]", *e, s);
}

sm_event *sm_state_pop_event(sm_state *s)
{
    if (s->ctl.E && SM_STATE_EVENT_TRACE(s) != NULL)
    {
        sm_event *e = SM_STATE_EVENT_TRACE(s);
        e->ctl.L = false;
        if (SM_STATE_EVENT_TRACE(s)->next != NULL)
        {
            SM_STATE_EVENT_TRACE(s) = SM_STATE_EVENT_TRACE(s)->next;
        }
        SM_DEBUG_MESSAGE("sm_event [addr:%p] successfully popped from stack of sm_state at [addr:%p]", e, s);
        return e;
    }
    else
    {
        return NULL;
        SM_DEBUG_MESSAGE("no sm_event available in the stack of sm_state at [addr:%p]", s);
    }
}

int sm_state_to_string(sm_state *s, char *buffer)
{
    char *pos = buffer;
    if (SM_UNLIKELY(s == NULL))
    {
        pos += sprintf(pos, "NULL\n");
    }
    else
    {
        char data_buffer[32];
        pos += sprintf(pos, "address: %p\n", s);
        pos += sprintf(pos, "**fsm: %p\n", s->fsm);
        if (s->fsm != NULL)
        {
            pos += sprintf(pos, "*fsm: %p\n", *(s->fsm));
            if (*s->fsm != NULL)
            {
                pos += sprintf(pos, "fsm: %s\n", (*(s->fsm))->name);
            }
        }
        pos += sprintf(pos, "service_id: %u\n", s->service_id);
        pos += sprintf(pos, "state_id: %u\n", s->state_id);
        pos += sprintf(pos, "size: %u B\n", s->ctl.size << 6);
        if (s->ctl.D)
        {
            pos += sprintf(pos, "depot addr: %p\n", SM_STATE_DEPOT(s));
            pos += sprintf(pos, "hash key addr: %p\n", SM_STATE_HASH_KEY(s));
            pos += sprintf(pos, "key string allocated in the data block: %u\n", s->ctl.K);
            pos += sprintf(pos, "key string: %s\n", (char *)SM_STATE_HASH_KEY(s)->string);
            pos += sprintf(pos, "key length: %d\n", SM_STATE_HASH_KEY(s)->length);
            pos += sprintf(pos, "key hash: 0x%X\n", SM_STATE_HASH_KEY(s)->value);
            pos += sprintf(pos, "next state address: %p\n", SM_STATE_NEXT(s));
        }
        if (s->ctl.E)
        {
            pos += sprintf(pos, "event trace head address: %p\n", SM_STATE_EVENT_TRACE(s));
        }
        if (s->ctl.T)
        {
            pos += sprintf(pos, "Tx object address: %p\n", SM_STATE_TX(s));
        }
        if (s->ctl.H)
        {
            pos += sprintf(pos, "handle: %p\n", SM_STATE_HANDLE(s));
        }
        if (s->ctl.size > 0)
        {
            pos += sprintf(pos, "data addr: %p\n", SM_STATE_DATA(s));
            snprintf(data_buffer, 32, "%s", (char *)SM_STATE_DATA(s));
            pos += sprintf(pos, "data:\n[%s ...]\n", data_buffer);
        }
    }
    return (int)(pos - buffer);
}

static inline size_t sm_state_sizeof(sm_state *s)
{
    return sizeof(sm_state) + s->ctl.D * (sizeof(sm_hash_key) + 2 * SM_WORD) + (s->ctl.E + s->ctl.T + s->ctl.H) * SM_WORD + SM_STATE_DATA_SIZE(s);
}