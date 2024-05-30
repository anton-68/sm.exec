#include <stdio.h>  // fprintf()
#include <stddef.h> // size_t, ptrdiff_t
#include <stdint.h> // uint8_t

int main() {
	typedef uint8_t sm_word_t;
	typedef sm_word_t * sm_word_ptr_t;
	printf("sizeof(sm_word_t) = %ld\n", sizeof(sm_word_t));
	printf("sizeof(sm_word_ptr_t) = %ld\n", sizeof(sm_word_ptr_t));
	sm_word_t x[] = {7, 11, 13, 17};
	sm_word_ptr_t p0 = x;
	printf("\n");
	for(ptrdiff_t i = 0; i <= 3; i++)
		printf("&x[%ld] = %p, x[%ld] = %d\n", i, p0 + i, i, *(p0 + i)); 
	unsigned u = 1;
	int i = -1;
	printf("\n");
	printf("sizeof(ptrdiff_t) = %ld\n", sizeof(ptrdiff_t));
	printf("sizeof(unsigned int) = %ld\n", sizeof(unsigned int));
	printf("sizeof(unsigned long int) = %ld\n", sizeof(unsigned long int));
	printf("sizeof(unsigned long long int) = %ld\n", sizeof(unsigned long long int));
	printf("sizeof(int) = %ld\n", sizeof(int));
	printf("sizeof(long int) = %ld\n", sizeof(long int));
	printf("sizeof(long long int) = %ld\n", sizeof(long long int));
	printf("\n");
	printf("1 > (int)(-1) = %d\n", 1 > i);
	printf("(int)(-1) < (unsigned)1 = %d\n", i < u);
	return 0;
}