#include <stdio.h>
#include <pthread.h>

int main() {
	printf("sizeof(pthread_mutex_t) = %ld\n", sizeof(pthread_mutex_t));
	printf("sizeof(pthread_cond_t) = %ld\n", sizeof(pthread_cond_t));
	return 0;
}