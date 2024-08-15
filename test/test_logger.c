/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Logger module tests
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "test_utils.h"

int main()
{
    SM_REPORT_MESSAGE(SM_LOG_INFO, "SM_REPORT_MESSAGE() macro test");
    SM_REPORT_CODE(SM_LOG_ERR, EINVAL);
    SM_SYSLOG(SM_CORE, SM_LOG_NOTICE, "mutex error was resolved", EXIT_SUCCESS);
    SM_DEBUG_MESSAGE("just debugging");
    SM_DEBUG_CODE(EXIT_FAILURE);
}