#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

int main() {
	printf("sizeof(pthread_mutex_t) = %ld\n", sizeof(pthread_mutex_t));
	printf("sizeof(pthread_cond_t) = %ld\n", sizeof(pthread_cond_t));
	printf("sizeof(time_t) = %ld\n", sizeof(time_t));
	printf("sizeof(long) = %ld\n", sizeof(long));
	printf("sizeof(unsigned int) = %ld\n", sizeof(unsigned int));
	return 0;
}