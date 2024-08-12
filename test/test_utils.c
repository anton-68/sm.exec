/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Test utils
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "test_utils.h"

void sm_print_hash_key(const sm_hash_key *k)
{
    printf("Hash_key\n=====\n");
    printf("string = %s\n", (const char *)k->string);
    printf("length = %u\n", k->length);
    printf("hash = %08X\n\n", k->value);
}

void sm_print_event(const sm_event *e)
{
    char buffer[SM_TEST_PRINT_BUFFER];
    sm_event_to_string(e, buffer);
    printf("Event\n=====\n%s\n", buffer);
    while (e != NULL && e->ctl.L)
    {
        e = e->next;
        sm_event_to_string(e, buffer);
        printf("\nLinked event\n=====\n%s\n\n", buffer);
    }
}

void sm_print_state(sm_state *s)
{
    char buffer[SM_TEST_PRINT_BUFFER];
    sm_state_to_string(s, buffer);
    printf("State\n=====\n%s\n", buffer);
    if (s != NULL && s->ctl.E) {
        sm_event *e = SM_STATE_EVENT_TRACE(s);
        do
        {
            sm_event_to_string(e, buffer);
            printf("Stack event\n===========\n%s\n", buffer);
        } while (e != NULL && (e = e->next) != NULL);
    }
}

void sm_print_queue(sm_queue *q)
{
    char buffer[SM_TEST_PRINT_BUFFER];
    sm_queue_to_string(q, buffer);
    printf("Queue\n=====\n%s\n", buffer);
}

void sm_print_queue2(sm_queue2 *q)
{
    char buffer[SM_TEST_PRINT_BUFFER];
    sm_queue2_to_string(q, buffer);
    printf("Queue2\n======\n%s\n", buffer);
}

void sm_print_pqueue(sm_pqueue *q)
{
    char buffer[SM_TEST_PRINT_BUFFER];
    sm_pqueue_to_string(q, buffer);
    printf("PQueue\n======\n%s\n", buffer);
}

void sm_print_array(sm_array *a)
{
    char buffer[SM_TEST_PRINT_BUFFER];
    sm_array_to_string(a, buffer);
    printf("Array\n=====\n%s\n", buffer);
}

void sm_print_directory(sm_directory *d)
{
    char buffer[SM_TEST_PRINT_BUFFER];
    sm_directory_to_string(d, buffer);
    printf("Directory\n=========\n%s\n", buffer);
    sm_directory_record *rec = d->top; 
    while (rec != NULL)
    {
        printf("Directory record\n");
        printf("item name: %s\n", rec->name);
        printf("item ptr: %p\n", rec->ptr);
        printf("item ref: %p\n", rec->ref);
        printf("next item rec addr: %p\n", rec->next);
        printf("prev item rec addr: %p\n", rec->prev);
        rec = rec->next;
    }
}

char sm_buffer[SM_FSM_PRINT_BUFFER];
void sm_print_fsm(sm_fsm *f, sm_directory *d)
{
    //char buffer[SM_FSM_PRINT_BUFFER];
    char *buffer = sm_fsm_to_string(f, d);
    printf("FSM\n===\n%s\n", buffer);
}

