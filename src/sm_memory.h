/* SM.EXEC
   Memory
   (c) anton.bondarenko@gmail.com */

#ifndef SM_MEMORY_H
#define SM_MEMORY_H

#include <stdint.h>     // uint32_t
#include "sm_sys.h"
#include "sm_event.h"

// simple memory manager

// sm_chunk
typedef struct sm_chunk {
	struct sm_chunk *next;
	struct sm_chunk *prev;
	SM_ID id;
	uint32_t size;
	void * data;
} sm_chunk; // Must be 32b!

// Macros
#define SM_CHUNK_DATA(C) (void *)((char *)(C) + sizeof(sm_chunk))
#define SM_CHUNK_HEAD(C) (sm_chunk *)((C)->data)
#define SM_CHUNK_LAST(C) ((sm_chunk *)(C))->prev
#define SM_CHUNK_LENGTH(C) ((char *)((C)->next) - (char *)(C) - sizeof(sm_chunk))

// Public methods
void sm_memory_init(void * data, size_t datasize);
// DEPRECATED -> moving to sm_sys and switching to macroses
void * sm_memory_align(void * addr, size_t align);
size_t sm_memory_size_align(size_t size, size_t align);
void * sm_memory_add_addr(void * addr, size_t add);
// END OF DEPRECATED
size_t sm_chunk_memory(sm_chunk *c);
void *sm_chunk_limit(sm_chunk *c);
sm_chunk *sm_chunk_find(void *data, SM_ID id);
sm_chunk *sm_chunk_open(void *data, SM_ID id);
sm_chunk *sm_chunk_next(sm_chunk *this);
sm_chunk *sm_chunk_prev(sm_chunk *this);

// Simple Read-Write-Allocate-Rewind-Skip serialization: 
void * sm_memory_write_int(sm_chunk *c, void ** start, int value);
void * sm_memory_read_int(sm_chunk *c, void ** start, int *value);
void * sm_memory_allocate_int_array(sm_chunk *c, void ** start, size_t size);
void * sm_memory_allocate_2d_int_array(sm_chunk *c, void ** start, size_t row, size_t col);
void * sm_memory_write_string(sm_chunk *c, void * const start, const char *str);
void * sm_memory_read_string(sm_chunk *c, void * const start, char *str);
void * sm_memory_check_string(sm_chunk *c, void * const start);
void * sm_memory_skip(sm_chunk *c, void * const start, size_t size, size_t align);

#endif //SM_MEMORY_H