/* SM.EXEC
   Tape: Simple tape recorder type serializer 
   (c) anton.bondarenko@gmail.com */

#ifndef SM_TAPE_H
#define SM_TAPE_H
#include <stdint.h>

typedef struct sm_tape_dcb {
	void *start;
	void *position;	
	void *end;
} sm_tape_dcb;

sm_tape_dcb *sm_tape_init(void *start, size_t size);
void sm_type_rewind(sm_tape_dcb *dcb);
int sm_tape_write_int(sm_tape_dcb *dcb, int value);
int sm_tape_read_int(sm_tape_dcb *dcb, int *value);
int* sm_tape_allocate_array(sm_tape_dcb *dcb, size_t size);
int** sm_tape_allocate_2d_array(sm_tape_dcb *dcb, size_t rows, size_t cols);
int sm_tape_write_string(sm_tape_dcb *dcb, const char *str);
int sm_tape_read_string(sm_tape_dcb *dcb, char *str);
void * sm_tape_ff(sm_tape_dcb *dcb, size_t size);

#endif //SM_TAPE_H