/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Event module tests
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "../src/sm_event.h"
#include "test_utils.h"

int main()
{
    // NULL event
    sm_event *e = NULL;
    SM_EVENT_DESTROY(e);

    // Minimal event
    e = sm_event_create(0, false, false, false, false);
    sm_print_event(e);
    SM_EVENT_DESTROY(e);

    // Stand-alone complete event & access header
    e = sm_event_create(256, false, true, true, true);
    strcpy(SM_EVENT_DATA(e), "Secret password ..............................");
    SM_EVENT_KEY_STRING(e) = SM_EVENT_DATA(e);
    SM_EVENT_KEY_LENGTH(e) = strlen(SM_EVENT_KEY_STRING(e));
    SM_EVENT_KEY_HASH(e) = 0xBCDE;
    SM_EVENT_PRIORITY_0(e) = 7;
    SM_EVENT_PRIORITY_1(e) = 17;
    SM_EVENT_HANDLE(e) = (void *)0xA123;
    sm_print_event(e);

    // Access data
    strcpy(SM_EVENT_DATA(e), "Test event data access");
    sm_print_event(e);

    // Clone
    sm_event *clone = sm_event_clone(e);
    strcpy(SM_EVENT_DATA(clone), "Test clone");
    SM_EVENT_DESTROY(e);
    sm_print_event(clone);
    SM_EVENT_DESTROY(clone);

    // Chaining
    e = sm_event_create(0, false, false, false, false);
    e->event_id = 77;
    sm_print_event(e);
    sm_event *e2 = sm_event_chain_end(e);
    sm_print_event(e2);
    sm_event *e1;
    for (uint16_t i = 0; i < 7; i++)
    {
        e1 = sm_event_create(256, false, false, false, false);
        e1->event_id = i;
        SM_EVENT_CHAIN(e, e1);
    }
    sm_print_event(e);
    SM_EVENT_DISPOSE(e);

    // e = sm_event_dispose(e);
    // sm_event_dispose(&e);
    // sm_event_chain(e0, &e1);
    // e1 = sm_event_chain(e0, e1);
    // e0 = sm_queue_enqueue(q0, e0);
    // sm_queue_enqueue(q0, &e0);

    SM_EVENT_DISPOSE(e1); // this will rase an error if chained with func
    sm_print_event(e);
    sm_print_event(e1);
}