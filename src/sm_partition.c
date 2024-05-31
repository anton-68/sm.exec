/* SM.EXEC
   Data block partitioning
   (c) anton.bondarenko@gmail.com */

#include <string.h>
#include "sm_partition.h"
#include "sm_sys.h"

#define _prev(D,P) ((sm_partition *)((sm_word_t *)(D) + (ptrdiff_t)(P)->prev))
#define _next(D,P) ((sm_partition *)((sm_word_t *)(D) + (ptrdiff_t)(P)->next))
#define _last(D) _prev((D),(sm_partition *)(D)) 

void sm_partition_init(void *data, size_t size){
	sm_partition *head = (sm_partition *)data;
	head->next = 0;
	head->prev = 0;
	head->id = 0;
	head->size = SM_CEILING(size, SM_WORD) - 2;
}

sm_partition *sm_partition_find(void *data, uint32_t id) {
	sm_partition *head = (sm_partition *)data;
	if(head->id == 0)
		return NULL; 
	while(head != NULL && head->id != id)
		head = _next(data, head);
	return head;
}

sm_partition *sm_partition_open(void *data, uint32_t id) {
	sm_partition *new = sm_partition_find(data, id);
	if(new != NULL)
		return new;
	if(sm_partition_data_size(data) <= sizeof(sm_partition))
		return NULL; 
	new = _last(data);
	new->id = id;
	sm_partition_check(data, new, new + (ptrdiff_t)1);
	_prev(data, new)->size = (sm_word_t *)_prev(data, new) - (sm_word_t *)new - 2;
	return new;
}

void sm_partition_check(void *data, sm_partition *this, void *edge) {
	if(this->next != 0 && _next(data, this)->id != 0)
		return;
	sm_partition * new_edge = (sm_partition *)SM_ALIGN(edge, SM_WORD);
	if(_next(data, this) > new_edge)
		return;
	size_t datasize = this->next + sm_partition_data_size(data) + 2;
	this->next = (uint32_t)((sm_word_t *)new_edge - (sm_word_t *)data);
	new_edge->next = 0;
	new_edge->prev = (uint32_t)((sm_word_t *)this - (sm_word_t *)data);
	new_edge->id = 0;
	new_edge->size = datasize - this->next - 2;
	this->size = new_edge->size + (sm_word_t *)new_edge - (sm_word_t *)this - 2;
	((sm_partition *)data)->prev = this->next;
}

size_t sm_partition_size(void *data, sm_partition *this){ 
	return this->size;
}

size_t sm_partition_data_size(void *data) {
	return sm_partition_size(data, _last(data));
}

size_t sm_partition_full_size(void *data) {
	return sm_partition_data_size(data) + ((sm_partition *)data)->prev + 2;
}

void sm_partition_delete(void *data, sm_partition *this){
	size_t fullsize = sm_partition_full_size(data);
	((sm_partition *)data)->prev = _prev(data, this)->next;
	this->id = 0;
	this->next = 0;
	this->size = fullsize - ((sm_partition *)data)->prev - 2;
}