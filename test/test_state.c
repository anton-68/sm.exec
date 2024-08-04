/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
State module tests
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "test_utils.h"

int main()
{
    openlog(NULL, LOG_NDELAY, LOG_USER);

    sm_state *s = sm_state_create(NULL, 0, NULL, false, false, false, false);
    sm_print_state(s);
    SM_STATE_DESTROY(s);

    char *fsm_name = "FSM0";
    sm_fsm fsm0;
    fsm0.name = fsm_name;
    fsm0.initial = 77;
    sm_fsm *fsm0p = &fsm0;
    sm_fsm **fsm0r = &fsm0p;
    s = sm_state_create(fsm0r, 0, NULL, false, false, false, false);
    sm_print_state(s);
    SM_STATE_DESTROY(s);

    fsm0.initial = 78;
    s = sm_state_create(fsm0r, 0, (struct sm_array *)0xDEADBEEF, false, false, true, false);
    sm_print_state(s);
    SM_STATE_DESTROY(s);

    fsm0.initial = 79;
    s = sm_state_create(fsm0r, 0, NULL, true, false, false, false);
    sm_print_state(s);
    SM_STATE_DESTROY(s);

    fsm0.initial = 80;
    s = sm_state_create(fsm0r, 256, (struct sm_array *)0xDEADBEEF, true, false, true, false);
    sm_print_state(s);

    char *state_data = "Data string ..................................................";
    strcpy((char *)SM_STATE_DATA(s), state_data);
    SM_STATE_DEPOT(s) = (void *)0xABCD;
    sm_hash_set_key(SM_STATE_HASH_KEY(s), SM_STATE_DATA(s), strlen((char *)SM_STATE_DATA(s)), 0xffffffff);
    //SM_STATE_HASH_KEY(s)->string = SM_STATE_DATA(s);
    //SM_STATE_HASH_KEY(s)->length = strlen(SM_STATE_HASH_KEY(s)->string);
    //SM_STATE_HASH_KEY(s)->value = 0xBCDE;
    SM_STATE_NEXT(s) = (void *)0xCDEF;
    SM_STATE_EVENT_TRACE(s) = NULL;
    SM_STATE_HANDLE(s) = (void *)0xA123;
    sm_print_state(s);

    state_data = "Test event data access";
    strcpy((char *)SM_STATE_DATA(s), state_data);
    sm_print_state(s);

    // push - pop

    sm_event *e0 = sm_event_create(0, false, false, false, false);
    //e0->ctl.L = true;

    sm_event *e1 = sm_event_create(256, false, true, true, true);
    strcpy(SM_EVENT_DATA(e1), "Secret password ..............................");
    sm_hash_set_key(SM_STATE_HASH_KEY(s), SM_STATE_DATA(s), strlen((char *)SM_STATE_DATA(s)), 0xffffffff);
    //SM_EVENT_KEY_STRING(e1) = SM_EVENT_DATA(e1);
    //SM_EVENT_KEY_LENGTH(e1) = strlen(SM_EVENT_KEY_STRING(e1));
    //SM_EVENT_KEY_HASH(e1) = 0xBCDE;
    SM_EVENT_PRIORITY(e1)[0] = 7;
    SM_EVENT_PRIORITY(e1)[1] = 17;
    SM_EVENT_HANDLE(e1) = (void *)0xA123;
    //e1->ctl.L = true;

    sm_event *clone = sm_event_clone(e1);
    strcpy(SM_EVENT_DATA(clone), "Clone ..............................");
    //clone->ctl.L = false;
    //clone->next = NULL;

    SM_STATE_PUSH_EVENT(s, e0);
    SM_STATE_PUSH_EVENT(s, e1);
    SM_STATE_PUSH_EVENT(s, clone);

    sm_print_state(s);

    //s->ctl.D = false;
    SM_STATE_DISPOSE(s);

    sm_print_state(s);
}