#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

typedef struct sm_partition {
	uint32_t next;
	uint32_t prev;
	uint32_t size;
	uint32_t id;
} sm_partition;

int main() {
	printf("sizeof(ptrdiff_t) = %ld\n", sizeof(ptrdiff_t)); 	
	sm_partition p;
	sm_partition *pptr = &p;
	printf("sm_partition *pptr = %p, pptr + 1 = %p\n", pptr, pptr + 1);
	return 0;
}