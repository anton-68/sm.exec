/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
FSM class - Refactor
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#ifndef SM_FSM_H
#define SM_FSM_H

#include "sm_event.h"

typedef struct sm_fsm
{
    sm_index *events;
    sm_index *apps;
    sm_index *states;
    uint16_t service_id;
    char *name;
} sm_fsm;

sm_fsm *
sm_fsm_create();

int 
sm_fsm_add_state(sm_fsm *sm, const char * const name, uint16_t id);








#endif // SM_FSM_H