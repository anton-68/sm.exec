/* SM.EXEC
   Memory
   (c) anton.bondarenko@gmail.com */

#include <stdint.h> // uint32_t
#include <string.h>

#include "sm_memory.h"

// Private methods

static void sm_id_to_ipstr(SM_ID id, char *const ip)
{
	sprintf(ip, "%d.%d.%d.%d", (id >> 24) & 0xFF, (id >> 16) & 0xFF, (id >> 8) & 0xFF,
			(id) & 0xFF);
}

static SM_ID sm_ipstr_to_id(const char *const ip)
{
	unsigned byte3, byte2, byte1, byte0;
	char dummyString[2];
	if (sscanf(ip, "%u.%u.%u.%u%1s",
			   &byte3, &byte2, &byte1, &byte0, dummyString) == 4)
	{
		if (byte3 < 256 && byte2 < 256 && byte1 < 256 && byte0 < 256)
		{
			SM_ID id = (byte3 << 24) + (byte2 << 16) + (byte1 << 8) + byte0;
			return id;
		}
	}
	return 0;
}

static void check(sm_chunk *t, void *e) {
	if(t->next != NULL && t->next->id != 0) // closed chunk cannot be reset
		return;
	sm_chunk *this = (sm_chunk *)t;  	   // this
	sm_chunk *head = (sm_chunk *)t->data;  // head
	sm_chunk *edge = sm_memory_align(e, sizeof(int *)); // new boundary
	if(this->next > edge)					// already large enough
		return;
	edge->next = NULL;
	edge->prev = this;
	edge->id = 0;
	edge->data = this->data;
	edge->size = this->size;
	
	//head->prev->next = edge;
	
	head->prev = edge;
	this->next = edge;
}

// Public methods

// CHUNKS

void sm_memory_init(void * data, size_t datasize){ // must be called first! 
	sm_chunk *c = (sm_chunk *)data;
	c->next = NULL;
	c->prev = c;
	c->id = 0;
	c->data = data;
	c->size = datasize;
}

size_t sm_chunk_memory(sm_chunk *c){
	if(c->id == 0) 			// empty
		return c->size - 2 * sizeof(sm_chunk);
	if(c->next->id == 0)	// open/last
		return (char *)c->data + c->size - (char *)c - 2 * sizeof(sm_chunk);
	return (char *)c->next - (char *)c - sizeof(sm_chunk);
}

void *sm_chunk_limit(sm_chunk *c){
	if(c->id == 0 || c->next->id == 0) // empty or last
		return (void *)((char *)c->data + c->size - sizeof(sm_chunk));
	return (void *)c->next; // closed
}

void *sm_data_limit(sm_chunk *c){
	if(c->id == 0) 			// empty
		return (void *)((char *)c->data + c->size - sizeof(sm_chunk));
	else					// open or closed
		return (void *)c->next;
}

sm_chunk *sm_chunk_find(void *data, SM_ID id){
	sm_chunk *c = (sm_chunk *)data;
	if(c->id == 0) // empty
		return NULL; 
	while(c != NULL && c->id != id)
		c = c->next;
	return c;
}

sm_chunk *sm_chunk_open(void *data, SM_ID id) {
	sm_chunk *new = sm_chunk_find(data, id);
	if(new != NULL)
		return new; // already open
	if(sm_chunk_memory(SM_CHUNK_LAST(data)) <= sizeof(sm_chunk)) 
		return NULL; // insufficient space
	new = ((sm_chunk *)data)->prev;
	new->id = id;
	check(new, SM_CHUNK_DATA(new));
	return new;
}
					  
sm_chunk *sm_chunk_next(sm_chunk * this){
	if(this == NULL || this->next == NULL || this->id == 0)
		return NULL;
	return this->next;
}
	
sm_chunk *sm_chunk_prev(sm_chunk *this){						 
	if(this->prev->next != NULL)
		return this->prev;
	else
		return NULL;
}

// Simple C <-> Lua serialization: Read-Write-Allocate-Rewind-Skip

void * sm_memory_align(void * addr, size_t align) {
	//return (void *)((size_t)(char *)addr / align * align + (size_t)(char *)addr % align);
	return 
		(void *)
		(   (size_t)(char *)addr / align * align + 
		   ((size_t)(char *)addr % align ? align : 0)   );
}

size_t sm_memory_size_align(size_t size, size_t align) {
	return size / align * align + size % align ? align : 0;
}

void * sm_memory_add_addr(void * addr, size_t add) {
	return (void *)((char *)addr + add);
}

void * sm_memory_write_int(sm_chunk *c, void ** start, int value){
	if(c == NULL || c->next == NULL || c->id == 0)
		return NULL; // misconfigured chunk
	*start = sm_memory_align(*start, sizeof(int));
	void *edge = sm_memory_add_addr(*start, sizeof(int));
	if(sm_chunk_limit(c) < edge)
		return NULL; // insufficient space
	*(int *)(*start) = value;
	check(c, edge);
	return edge;
}

void * sm_memory_read_int(sm_chunk *c, void ** start, int *value){
	if(c == NULL || c->next == NULL || c->id == 0)
		return NULL; // misconfigured chunk
	*start = sm_memory_align(*start, sizeof(int));
	void *edge = sm_memory_add_addr(*start, sizeof(int));
	if(sm_data_limit(c) < edge)
	   return NULL; // trying top read outiside of allocate chunk space
	if(sm_memory_align(*start, sizeof(int)) != *start)
		return NULL; // misaligned data address
	*value = *(int *)(*start);
	return edge;
}

void * sm_memory_allocate_int_array(sm_chunk *c, void ** start, size_t size){
	if(c == NULL || c->next == NULL || c->id == 0)
		return NULL; // misconfigured chunk
	*start = sm_memory_align(*start, sizeof(int));
	void* edge = sm_memory_add_addr(*start, sizeof(int) + sizeof(int) * size);
	if(sm_chunk_limit(c) < edge)
	   return NULL; // insufficient space
	*(int *)(*start) = (int)size;
	*start = sm_memory_add_addr(*start, sizeof(int));
	check(c, edge);
	return edge;
}

void * sm_memory_allocate_2d_int_array(sm_chunk *c, void ** start, size_t row, size_t col){
	if(c == NULL || c->next == NULL || c->id == 0)
		return NULL; // misconfigured chunk
	*start = sm_memory_align(*start, sizeof(int *));
	void* edge = sm_memory_add_addr(*start, sizeof(int *) * row + sizeof(int) * (2 + col * row));
	if(sm_chunk_limit(c) < edge)
	   return NULL; // insufficient space
	((int *)(*start))[0] = (int)row;
	((int *)(*start))[1] = (int)col;
	*start = sm_memory_add_addr(*start, sizeof(int) * 2);
	check(c, edge);
	for(int i = 0; i < row; i++)
		((int **)(*start))[i] = (int *)((char *)(*start) + row * sizeof(int *) + i * col * sizeof(int));
	return edge;
}

void * sm_memory_write_string(sm_chunk *c, void * const start, const char *str){
	if(c == NULL || c->next == NULL || c->id == 0)
		return NULL; // misconfigured chunk
	void* edge = sm_memory_add_addr(start, strlen(str) + 1);
	if(sm_chunk_limit(c) < edge)
	   return NULL; // insufficient space
	check(c, edge);						 			 
	memcpy(start, str, strlen(str));
	return edge;
}

void * sm_memory_read_string(sm_chunk *c, void * const start, char *str){
	if(c == NULL || c->next == NULL || c->id == 0)
		return NULL; // misconfigured chunk
	void* edge = sm_memory_add_addr(start, strlen((char *)start) + 1);
	if(sm_data_limit(c) < edge)
	   return NULL; // oversized string (missing /0)
	memcpy(str, start, strlen(start));
	return edge;
}

void * sm_memory_check_string(sm_chunk *c, void * const start/*, char *str*/){
	if(c == NULL || c->next == NULL || c->id == 0)
		return NULL; // misconfigured chunk
	void* edge = sm_memory_add_addr(start, strlen((char *)start) + 1);
	if(sm_data_limit(c) < edge)
	   return NULL; // oversized string (missing /0)
	return edge;
}
	
void * sm_memory_skip(sm_chunk *c, void * const start, size_t size, size_t align) {
	if(c == NULL || c->next == NULL || c->id == 0)
		return NULL; // misconfigured chunk
	void* edge = sm_memory_add_addr(sm_memory_align(start, align), size);
	if(sm_chunk_limit(c) < edge)
	   return NULL; // insufficient space
	return edge;
}
