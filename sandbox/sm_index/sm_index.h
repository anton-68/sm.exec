/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Index data type class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#ifndef SM_INDEX_H
#define SM_INDEX_H

#include <stdint.h>
#include "sm_hash.h"

typedef struct sm_index
{
    //
} sm_index;

sm_index *
sm_index_create(uint8_t size, bool hashed);

void
sm_index_destroy(sm_index *i);

int
sm_index_add(const char *const name, uint16_t id);

int 
sm_index_delete(const char *const name);

int 
sm_index_delete_by_id(uint16_t id);

uint16_t 
sm_index_get(const char *const name);

char *
sm_index_get_by_id(uint16_t id);

#endif // SM_FSM_H