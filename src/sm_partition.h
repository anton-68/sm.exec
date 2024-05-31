/* SM.EXEC
   Data block partitioning
   (c) anton.bondarenko@gmail.com */

#ifndef SM_PARTITION_H
#define SM_PARTITION_H
#include <stdint.h>

typedef struct sm_partition {
	uint32_t id;
	uint32_t next;
	uint32_t prev;
	uint32_t size;
} sm_partition;

#define SM_PARTITION_DATA(P) (void *)((sm_partition *)(P)+(ptrdiff_t)1)

void sm_partition_init(void *data, size_t datasize);
sm_partition *sm_partition_find(void *data, uint32_t id);
sm_partition *sm_partition_open(void *data, uint32_t id);
void sm_partition_check(void *data, sm_partition *this, void *edge);
size_t sm_partition_size(void *data, sm_partition *this);
size_t sm_partition_data_size(void *data);
void sm_partition_delete(void *data, sm_partition *this);

#endif //SM_PARTITION_H
