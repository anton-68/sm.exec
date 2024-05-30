#include <stdio.h>

int * triple(int * p) {
	p = (int *)((unsigned long long)p * 3);
	return p;
}

int main() {
	int q = 77;
	printf("triple(%p) = %p\n", &q, triple(&q));
}

// 0x07ffe709a72d4  
// 0x17ffb51cf587c
// passed
