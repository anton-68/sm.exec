/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Hash module tests
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "test_utils.h"

int main()
{
    char key0[] = "test_key_0";
    char key1[] = "test_key_1";
    sm_hash_key k0, k1;
    sm_print_hash_key(&k0);
    sm_print_hash_key(&k1);
    sm_hash_set_key(&k0, key0, strlen(key0), 0xffffffff);
    sm_print_hash_key(&k0);
    sm_hash_set_key(&k1, key1, strlen(key1), 0xffffffff);
    sm_print_hash_key(&k1);
    printf("sm_hash_key_match(&k0, &k1) = %u\n", sm_hash_key_match(&k0, &k1));
    printf("sm_hash_key_match(&k0, &k0) = %u\n", sm_hash_key_match(&k0, &k0));
}