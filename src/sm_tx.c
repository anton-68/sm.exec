/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Thread-worker descriptor class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "sm_tx.h"

__thread void *__sm_tx_desc = (void *)0xCAFEBABE; 

sm_tx *sm_tx_create(sm_exec *exec,
                    sm_fsm **f,
                    size_t size,
                    size_t state_size,
                    sm_queue2 **q,
                    bool synchronized)
{
    sm_tx *t;
    if ((t = malloc(sizeof(sm_tx))) == NULL)
    {
        //SM_REPORT_MESSAGE(SM_LOG_ERR, "malloc() failed");
        SM_SYSLOG_CAUSE(SM_CORE, SM_LOG_ERR, "Cannot allocate thread descriptor", "malloc() returned NULL");
        return NULL;
    }
    t->exec = exec;
    if (q == NULL)
    {
        free(t);
        SM_SYSLOG_CAUSE(SM_CORE, SM_LOG_ERR, "Cannot create thread descriptor", "Input queue pointer is NULL");
        return NULL;
    }
    t->input_queue = q;
    t->synchronized = synchronized;
    if ((t->state = sm_state_create(f, state_size, NULL, false, true, false, false)) == NULL)
    {
        free(t);
        SM_SYSLOG_CAUSE(SM_CORE, SM_LOG_ERR, "Cannot create thread default state object", "sm_queue_create() returned NULL");
        return NULL;
    }
    //SM_STATE_DEPOT(t->state) = NULL;
    SM_STATE_TX(t->state) = t;
    t->default_state = t->state;
    t->data_size = t->data_block_size = size;
    if ((t->data = malloc(size + sizeof(sm_state *))) == NULL)
    {
        free(t->state);
        free(t);
        SM_SYSLOG_CAUSE(SM_CORE, SM_LOG_ERR, "Cannot allocate data block for the state object", "malloc() returned NULL");
        return NULL;
    }
    *SM_TX_STACK_POINTER(t) = NULL;
    SM_DEBUG_MESSAGE("sm_tx at [addr:%p] successfully created", t);
    return t;
}

void sm_tx_destroy(sm_tx **tx)
{
    if (tx == NULL || *tx == NULL)
    {
        SM_SYSLOG_CAUSE(SM_CORE, SM_LOG_ERR, "Cannot deallocate the thread descriptor", "NULL pointer provided");
        return;
    }
    if ((*tx)->state == NULL)
    {
        SM_SYSLOG_CAUSE(SM_CORE, SM_LOG_ERR, "Cannot deallocate the state object", "NULL pointer provided");
        return;
    }
    else
    {
        sm_state_destroy(&(*tx)->state);
    }
    if ((*tx)->data == NULL)
    {
        SM_SYSLOG_CAUSE(SM_CORE, SM_LOG_ERR, "Cannot deallocate the state object", "NULL pointer provided");
        return;
    }
    else
    {
        free((*tx)->data);
    }
    free(*tx);
    SM_DEBUG_MESSAGE("sm_tx at [addr:%p] successfully destroyed", *tx);
    *tx = NULL;
}

// Thread-runner app
void *sm_tx_runner(void *arg)
{
    sm_tx *tx = (sm_tx *)arg;
    __sm_tx_desc = (void *)tx;
    sm_event *event = NULL;
    struct timespec req, rem;
    req.tv_sec = (time_t)(SM_SPINLOCK_NS / (long)10e9);
    req.tv_nsec = (long)(SM_SPINLOCK_NS % (long)10e9);
    if (tx->synchronized)
    {
        for (;;)
        {
            event = sm_lock_dequeue2(*tx->input_queue);
            sm_apply_event(event, tx->state);
            if (event->ctl.D)
            {
                sm_event_dispose(&event);
            }
            tx->state = tx->default_state;
            // if(tx->state->id == (*tx->state->fsm)->final)
            //     tx->state->id = (*tx->state->fsm)->initial;
        }
    }
    else
    {
        for (;;)
        {
            while ((event = sm_dequeue2(*tx->input_queue)) != NULL)
            {
                sm_apply_event(event, tx->state);
                if (event->ctl.D)
                {
                    sm_event_dispose(&event);
                }
                tx->state = tx->default_state;
                // if(tx->state->id == (*tx->state->fsm)->final)
                //     tx->state->id = (*tx->state->fsm)->initial;
            }
            nanosleep(&req, &rem);
        }
    }
    return 0;
}

int sm_tx_push_state(sm_tx *tx, sm_state **s)
{
    if (tx->data_size < sizeof(sm_state *))
        return EXIT_FAILURE;
    tx->data_size -= sizeof(sm_state *);
    *SM_TX_STACK_POINTER(tx) = tx->state;
    tx->state = *s;
    SM_STATE_TX(*s) = __sm_tx_desc;
    *s = NULL;
    return EXIT_SUCCESS;
}

int sm_tx_pop_state(sm_tx *tx)
{
    tx->state = *SM_TX_STACK_POINTER(tx);
    tx->data_size += sizeof(sm_state *);
    SM_STATE_TX(tx->state) = __sm_tx_desc;
    return EXIT_SUCCESS;
}

bool sm_tx_stack_empty(sm_tx *tx)
{
    return tx->data_size == tx->data_block_size;
}

int sm_tx_to_string(sm_tx *tx, char *buffer)
{
    char *s = buffer;
    if (SM_UNLIKELY(tx == NULL))
    {
        s += sprintf(s, "NULL\n");
    }
    else
    {
        s += sprintf(s, "address: %p\n", tx);
        s += sprintf(s, "tx data size: %lu\n", tx->data_size);
        s += sprintf(s, "tx data block size: %lu\n", tx->data_block_size);
        s += sprintf(s, "data block address: %p\n", tx->data);
        s += sprintf(s, "synchronized: %u\n", tx->synchronized);
        s += sprintf(s, "current state address: %p\n", tx->state);
        s += sprintf(s, "default state address: %p\n", tx->default_state);
    }
    return (int)((char *)s - (char *)buffer);
}