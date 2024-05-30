/* SM.EXEC
   Memory
   (c) anton.bondarenko@gmail.com */

#ifndef SM_MEMORY_H
#define SM_MEMORY_H

#include <stdint.h>     // uint32_t
#include "sm_sys.h"
#include "sm_event.h"




// sm_chunk
typedef struct sm_chunk {
	struct sm_chunk *next;
	struct sm_chunk *prev;
	SM_ID id;
	void * data;
	unsigned * size; 
} sm_chunk;

#define SM_CHUNK_DATA(C) (void *)((char *)(C) + sizeof(sm_chunk))
#define SM_CHUNK_HEAD(C) (sm_chunk *)((C)->data)
#define SM_CHUNK_LAST(D) ((sm_chunk *)(D))->prev
//#define SM_LAST_CHUNK(P) ((sm_chunk *)(P->data))->prev
//#define SM_CHUNK_LENGTH(P) ((char *)((P)->next) - (char *)(P) - sizeof(sm_chunk))
//#define SM_PREV_CHUNK(T) ((void *)((char *)(((sm_chunk *)(T))->prev) + sizeof(sm_chunk)))
//#define SM_PREV_CHUNK_ID(T) (((sm_chunk *)(T))->prev->id)


// Public methods

void sm_memory_init(void * data, size_t datasize);
size_t sm_chunk_memory(sm_chunk *c);
void *sm_chunk_limit(sm_chunk *c);
sm_chunk *sm_chunk_find(void *data, SM_ID id);
sm_chunk *sm_chunk_open(void *data, SM_ID id);
sm_chunk *sm_chunk_next(sm_chunk * this);
sm_chunk *sm_chunk_prev(sm_chunk *this);
void * sm_addr_align(void * addr, size_t align); //??
void * sm_add_addr(void * addr, size_t add);	 //??
void * sm_memory_write_int(sm_chunk *c, void * const start, int value);
void * sm_memory_allocate_int_array(sm_chunk *c, void * const start, size_t size);
void * sm_memory_allocate_2d_int_array(sm_chunk *c, void * const start, size_t row, size_t col);
void * memory_sm_write_string(sm_chunk *c, void * const start, char *str);
void * sm_memory_skip(sm_chunk *c, void * const start, size_t size, size_t align);

#endif //SM_MEMORY_H