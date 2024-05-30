/* SM.EXEC
   anton.bondarenko@gmail.com 
   Hash function by Bob Jenkins: 
   http://burtleburtle.net/bob/c/lookup3.c */

#ifndef SM_HASH_H
#define SM_HASH_H

#include <stdint.h>
#include <stddef.h>

typedef struct sm_hash_key {
	void * key;
	uint32_t * key_length;
	uint32_t * key_hash;
} sm_hash_key;

#define SM_HASH_SIZE(n) ((uint32_t)1<<(n))
#define SM_HASH_MASK(n) (SM_HASH_SIZE(n)-1)

uint32_t sm_hash_bj_little( const void *key, size_t length, uint32_t initval);

#endif //SM_HASH_H