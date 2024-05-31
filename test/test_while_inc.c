#include <stdio.h>

int main() {
	
int pool_size = 13;
	
while(--pool_size) {
	printf("Loop start, pool_size = %d\n", pool_size);
	if(pool_size == 1)
		printf("Loop rounds, pool_size = [1]\n");
	else
		printf("Loop rounds, pool_size = %d\n", pool_size);
	}
	return 0;
}