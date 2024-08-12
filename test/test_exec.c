/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
SM Executor module tests
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "test_utils.h"

int main()
{
    sm_directory *d = sm_directory_create();
    sm_exec *e = sm_exec_create(256, d);
    sm_print_exec(e);
    sm_exec_destroy(&e);
    sm_print_exec(e);
    sm_print_directory(d);
}