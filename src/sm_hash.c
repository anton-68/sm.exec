/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Hash key class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include <string.h> //memcmp
#include "sm_hash.h"

void sm_hash_set_key(sm_hash_key *dst, void *str, size_t len, uint32_t hash_mask)
{
    if (dst->string != NULL)
    {
        if (dst->length != 0)
        {
            dst->length = MIN(dst->length, len);
            memcpy(dst->string, str, dst->length);
        }
        else
        {
            dst->length = len;
            memcpy(dst->string, str, len);
        }
    }
    else
    {
        dst->string = str;
        if (dst->length ==0)
        {
            dst->length = len;
        }
        else
        {
            dst->length = MIN(dst->length, len);
        }
    }
    dst->value = hashlittle(dst->string, dst->length, 0) & hash_mask;
}

bool sm_hash_key_match(sm_hash_key *k0, sm_hash_key *k1)
{
    if(k0->length != k1->length)
        return false;
    else
        return (memcmp(k0->string, k1->string, k0->length) == 0);
}
    