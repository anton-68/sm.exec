#include <stdio.h>
#include <stdint.h>

typedef struct map {
	uint32_t id;
	struct {
		unsigned int next	: 15;
		unsigned int prev	: 15;
		unsigned int scale	:  2; 
	} ctl;
} map;

int main() {
	map q;
	q.id = 7;
	q.ctl.next = 3;
	q.ctl.prev = 5;
	q.ctl.scale = 2;
	printf("5 << 3 * 2 = %d\n", q.ctl.prev << 3 * q.ctl.scale);
	printf("map.ctl.prev(5) << 3 * map.ctl.scale(2) = %d\n", q.ctl.prev << 3 * q.ctl.scale);
	return 0;
}