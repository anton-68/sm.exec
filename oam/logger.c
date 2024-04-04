/* SM.EXEC
   Logger functions
   anton.bondarenko@gmail.com */

#include <stdio.h>
#include <errno.h>
#include <string.h>			// strerror()
#include <stdlib.h>			// EXIT_SUCCESS
#include "../src/sm_sys.h"
#include "logger.h"





//////// report

int report(severity_t severity, const char *message, const char *file, int line, const char *function) {
    if(severity == ERROR) 
        fprintf (stderr, "[    ERROR ] %s\n", strerror(errno)); 
    else
        fprintf (stderr, "[    EVENT ] %s\n", strerror(errno));
	char tid[32];
	get_tid_str(tid);
	fprintf (stderr, "[   THREAD ] %s\n", tid);
    fprintf (stderr, "[       IN ] %s\n", message);
    fprintf (stderr, "[     FILE ] %s\n", file);
    fprintf (stderr, "[     LINE ] %i\n", line - TL_REPORT_LN_ADJUSTMENT);
    fprintf (stderr, "[ FUNCTION ] %s\n\n", function);  
    return EXIT_SUCCESS;
}
