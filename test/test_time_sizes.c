#include <stdio.h>
#include <time.h>
#include <stdbool.h>

int main() {
	printf("sizeof(struct timespec) = %ld\n", sizeof(struct timespec));
	printf("sizeof(time_t) = %ld\n", sizeof(time_t));
	printf("sizeof(long) = %ld\n", sizeof(long));
	printf("sizeof(long long) = %ld\n", sizeof(long long));
	printf("sizeof(struct 1) = %ld\n", sizeof(
	 	struct {
			size_t capacity;
			size_t size;
			bool synchronized;
		}));
	printf("sizeof(struct 2) = %ld\n", sizeof(
	 	struct {
			size_t capacity;
			size_t size;
			bool synchronized;
			struct {
				unsigned int use_low_flag		: 1;
				unsigned int use_high_flag		: 1;
				unsigned int /* 28 bits */		: 0;	
			} ctl;
		}));
	printf("sizeof(bool) = %ld\n", sizeof(bool));
	return 0;
}