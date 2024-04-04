/* SM.EXEC
   Core functions tests
   anton.bondarenko@gmail.com */

#include <stdio.h>
#include "../../src/sm_sys.h"
#include "../../oam/logger.h"




/* worker */

void *worker_f (void* t_data) {
    //fprintf (stdout, "[    DEBUG ] tid of the thread:  %lu\n", get_tid());
	REPORT(EVENT, "worker_f()");
    pthread_exit("SUCCESS");
}





/* driver */

int main(void)
{   
    //fprintf (stdout, "[    DEBUG ] tid of the process: %lu\n", get_tid());
	fprintf(stdout,
		"test0\n"
		"test1\n"
		"test2\n"
	);
	fflush(stdout);
	REPORT(EVENT, "main()");
    pthread_t tx0, tx1, tx2;
    pthread_create(&tx0, NULL, worker_f, NULL);
    pthread_create(&tx1, NULL, worker_f, NULL);
    pthread_create(&tx2, NULL, worker_f, NULL);
    pthread_join(tx0, NULL);
    pthread_join(tx1, NULL);
    pthread_join(tx2, NULL);
    return 0;
}