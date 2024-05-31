#include <stdio.h>
#include <gnu/libc-version.h>
//#include <threads.h>
int main (void) { 
	_Atomic(int) i = 7;
	puts (gnu_get_libc_version ());
	printf("_Atomic(7) = %d\n", i);
	static _Thread_local int j = 99;
	printf("_Thread_local(99) = %d\n", j);
	return 0; 
}