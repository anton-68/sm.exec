/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Event class tests
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include <stdio.h>
#include "../src/sm_sys.h"

int main()
{
    sm_timestamp timestamp = sm_get_timestamp();
    printf("timestring = %s\n", timestamp.timestring);
    char buffer[80]; 
    get_tid_str(buffer);
    printf("tid (unsigned long) = %lu\ntid (string) = %s\n", get_tid(), buffer);
}