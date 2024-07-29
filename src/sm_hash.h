/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Hash key class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#ifndef SM_HASH_H
#define SM_HASH_H

#include "sm_logger.h"
#include "../lib/bj_hash/bj_hash.h"

/*
sm_hash - LP64
==============

 0         1          2         3          4         5          6
 0123456789012345 6789012345678901 23456789012345678901234567 890123
+-------------------------------------------------------------------+
|                          key string addr                          |
+---------------------------------+---------------------------------+
|            key length           |              key hash           |
+---------------------------------+---------------------------------+

sm_state - ILP32
================

 0         1         2          3
 0123456789012345 6789012345 678901
+----------------------------------+
|         key string addr          |
+----------------------------------+
|            key length            |
+----------------------------------+
|             key hash             |
+----------------------------------+
*/

#define SM_HASH_FUNC &hashlittle

typedef struct __attribute__((aligned(SM_WORD))) sm_hash_key
{
    void *string;
    uint32_t length;
    uint32_t value;
} sm_hash_key;

void sm_hash_set_key(sm_hash_key *dst, void *key_str, size_t key_len, uint32_t hash_mask);
bool sm_hash_key_match(sm_hash_key *k0, sm_hash_key *k1);

#endif // SM_HASH_H
