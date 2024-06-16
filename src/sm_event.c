/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Event class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "sm_event.h"

// Private methods

static inline size_t sm_event_sizeof(sm_event *e)
{
    return sizeof(sm_event) + SM_WORD * (e->ctl.Q + e->ctl.K + 
                                         e->ctl.P + e->ctl.H * 2)
                            + 8 * e->ctl.K
                            + SM_EVENT_DATA_SIZE(e);
}


// Public methods

sm_event *sm_event_copy(sm_event *e) 
{
    sm_event *new_e;
    if ((new_e = (sm_event *)malloc(sm_event_sizeof(e))) == NULL)
    {
        SM_REPORT(SM_LOG_ERR, "malloc() returned NULL");
        return e;
    }
    memset(new_e, 0, sm_event_sizeof(e));
    new_e->next = NULL;
    new_e->app_id = e->app_id;
    new_e->event_id = e->event_id;
    new_e->type = e->type;
    SM_REPORT(SM_LOG_DEBUG, "sm_event copied successfully");
    return new_e;
}

sm_event *sm_event_create(uint32_t size, bool Q, bool K, bool P, bool H) 
{
    sm_event header;
    header.next = NULL;
    header.app_id = 0;
    header.event_id = 0;
    header.ctl.size = size >> 6;
    header.ctl.Q = Q;
    header.ctl.K = K;
    header.ctl.P = P;
    header.ctl.H = H;
    header.ctl.L = false;
    header.ctl.D = true;
    SM_REPORT(SM_LOG_DEBUG, "sm_event created successfully");
    return sm_event_copy(&header);
}

void sm_event_free(sm_event *e)
{
    free(e);
    e = NULL;
} 

void sm_event_wipe(sm_event *e)
{
    uint32_t type = e->type;
    memset(e, 0, sm_event_sizeof(e));
    e->type = type;
}

int sm_event_to_string(sm_event *e, char *buffer) {
    char *s = buffer;
    char data_buffer[32];
    s += sprintf(s, "address: %p\n", e);
    s += sprintf(s, "next: %p\n", e->next);
    s += sprintf(s, "app_id: %u\n", e->app_id);
    s += sprintf(s, "event_id: %u\n", e->event_id);
    s += sprintf(s, "size: %u B\n", e->ctl.size * 64);
    if(e->ctl.Q)
        s += sprintf(s, "depot addr: %p\n", SM_EVENT_DEPOT(e));
    if (e->ctl.K) 
    {
        s += sprintf(s, "key string: %s\n", SM_EVENT_KEY_STRING(e)  != NULL ?
                                                 SM_EVENT_KEY_STRING(e) : "\0");
        s += sprintf(s, "key length: %d\n", SM_EVENT_KEY_LENGTH(e));
        s += sprintf(s, "key hash: 0x%X\n", SM_EVENT_KEY_HASH(e));
    }
    if (e->ctl.P) 
    {
        s += sprintf(s, "priority[0]: %lu\n", SM_EVENT_PRIORITY_0(e));
        s += sprintf(s, "priority[1]: %lu\n", SM_EVENT_PRIORITY_1(e));
    }
    if (e->ctl.H)
        s += sprintf(s, "handle: %p\n", SM_EVENT_HANDLE(e));
    s += sprintf(s, "linked: %s\n", e->ctl.L ? "true" : "false");
    s += sprintf(s, "disposable: %s\n", e->ctl.D ? "true" : "false");
    
    if (e->ctl.size > 0) {
        snprintf(data_buffer, 32, "%s", (char *)SM_EVENT_DATA(e));
        s += sprintf(s, "data:\n[%s ...]\n", data_buffer);
    }
    return (int)((char *)s - (char *)buffer);
}
