/* SM.EXEC
   Core functions tests
   anton.bondarenko@gmail.com */

#include <stdio.h>
#include "../../src/sm_sys.h"


/* driver */

int main(void)
{   
	
	int i = 0;
    struct timespec sleeptime = {0, 5000000L};

    sm_timestamp timestamp;

    for (i=0; i < 20; i++)
    {
        timestamp = sm_get_timestamp();
        printf("Time is: %s \n", timestamp.timestring);
        nanosleep(&sleeptime, NULL);
    }

    return 0;
}