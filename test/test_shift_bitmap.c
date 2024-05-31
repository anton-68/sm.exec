#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


typedef struct sm_partition {
	uint32_t id;
	uint32_t next;
	uint32_t prev;
	uint32_t size;
} sm_partition;

int main() {
	sm_partition *p0 = (sm_partition *)malloc(sizeof(sm_partition));
	sm_partition *p1 = (sm_partition *)malloc(sizeof(sm_partition));
	sm_partition *p2 = (sm_partition *)((char *)p1 + 24);
	printf("p0 = %p, p1 = %p, p2 = %p, p1-p0 = %ld, p2-p1 = %ld\n", p0, p1, p2, p1-p0, p2-p1);
	return 0;
}