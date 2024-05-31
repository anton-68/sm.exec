#include <stdio.h>
#include <gnu/libc-version.h>
#include <stdatomic.h>

typedef struct array {
	_Atomic struct {
		unsigned int taken 				:  1;
		unsigned int i					:  4;
		unsigned int /* reserved */ 	:  0;
	} ctl;
	int j;
} array;

int main (void) { 
	
	array i[2];
	i[0].ctl.i = 7;
	i[1].ctl.i = 11;
	
	printf("_Atomic array.ctl[0] = %d\n", i[0].ctl.i);
	printf("_Atomic array.ctl[1] = %d\n", i[1].ctl.i);
	
	printf("sizeof(atomic_flag) = %ld\n", sizeof(atomic_flag));
	
	return 0; 
}