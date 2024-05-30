#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct sm_event {
	struct sm_event *next; 					// 64
	uint32_t id;							// 32
	struct {
		unsigned int data_size /* words */ 	 : 16;
		unsigned int data_offset 		     :  8;
		unsigned int is_disposable 			 :  1;
		unsigned int hash_key_is_present 	 :  1;
		unsigned int home_address_is_present :  1;
		unsigned int home_is_tx_queue 		 :  1;
		unsigned int priority_is_present  	 :  1;
		unsigned int /* reserved - 3 bits */ :  0;
	} ctl;
} sm_event; 

long long pri[] = {0, 1};

long long * access_pri(void * p) {
	return (long long *)p;
}

int main() {
	//printf("sizeof(sm_event) = %ld\n", sizeof(sm_event));
	access_pri((void *)pri)[0] = 7;
	printf("pri[0] = %lld\n", access_pri((void *)pri)[0]);
	return EXIT_SUCCESS;
}