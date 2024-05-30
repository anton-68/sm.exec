#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "sm_logger.h"

int main() {
	SM_SYSLOG(SM_CORE, SM_LOG_ERR, "something went wrong ..");
	return EXIT_SUCCESS;
}