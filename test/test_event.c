/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Sys module tests
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../src/sm_event.h"

    int main() {
    char b[1024];
    sm_event *e = sm_event_create(0, false, false, false, false);
    sm_event_to_string(e, b);

    printf("Event\n=====\ntype: %u\n%s\n", e->type, b);

    sm_event_free(e);
    e = sm_event_create(256, true, true, true, true);
    strcpy(SM_EVENT_DATA(e), "Secret password ..............................");
    SM_EVENT_KEY_STRING(e) = SM_EVENT_DATA(e);
    SM_EVENT_KEY_LENGTH(e) = strlen(SM_EVENT_KEY_STRING(e));
    SM_EVENT_KEY_HASH(e) = 0xBCDE;
    SM_EVENT_PRIORITY_0(e) = 7;
    SM_EVENT_PRIORITY_1(e) = 17;
    SM_EVENT_HANDLE(e) = (void *)0xA123;
    sm_event_to_string(e, b);

    printf("Event\n=====\ntype: %u\n%s\n", e->type, b);

    sm_event *e_copy = sm_event_copy(e);
    strcpy(SM_EVENT_DATA(e_copy), "Test event copy");
    sm_event_to_string(e_copy, b);

    printf("Event\n=====\ntype: %u\n%s\n", e_copy->type, b);

    sm_event_free(e);
    sm_event_free(e_copy);
}