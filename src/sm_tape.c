/* SM.EXEC
   Tape: Simple tape recorder type serializer 
   (c) anton.bondarenko@gmail.com */

#include <stdlib.h>
#include <string.h>
#include "sm_sys.h"
#include "sm_logger.h"
#include "sm_tape.h"

sm_tape_dcb *sm_tape_init(void *start, size_t size) {
	sm_tape_dcb *dcb = (sm_tape_dcb *)start;
	dcb->position = dcb->start = SM_OFFSET(void, start, sm_tape_dcb, 1);
	dcb->end = SM_OFFSET(void, start, char, size);
	return (void *)dcb;
}

void sm_type_rewind(sm_tape_dcb *dcb){
	dcb->position = dcb->start;	
}

int sm_tape_write_int(sm_tape_dcb *dcb, int value) {
	void *tmp = (void *)SM_ALIGN(dcb->position, sizeof(int));
	void *end = SM_OFFSET(void, dcb->position, int, 1);
	if(end < dcb->end) {
		dcb->position = tmp;
		*(int *)dcb->position = value;
		dcb->position = end;
		return EXIT_SUCCESS; 
	}
	else {
		SM_LOG(SM_CORE, SM_LOG_DEBUG, "Insufficient space on the tape, failed to write int");
		return EXIT_FAILURE;
	}
}

int sm_tape_read_int(sm_tape_dcb *dcb, int *value) {
	void *tmp = (void *)SM_ALIGN(dcb->position, sizeof(int));
	void *end = SM_OFFSET(void, dcb->position, int, 1);
	if(end < dcb->end) {
		dcb->position = tmp;
		*value = *(int *)dcb->position;
		dcb->position = end;
		return EXIT_SUCCESS; 
	}
	else {
		SM_LOG(SM_CORE, SM_LOG_DEBUG, "Incorrect tape position, failed to read int");
		return EXIT_FAILURE;
	}
}

int *sm_tape_allocate_array(sm_tape_dcb *dcb, size_t size) {
	void *tmp = (void *)SM_ALIGN(dcb->position, sizeof(int));
	void *end = SM_OFFSET(void, dcb->position, int, (size + 1));
	if(end < dcb->end) {
		sm_tape_write_int(dcb, size);
		tmp = dcb->position;
		dcb->position = end;
		return (int *)tmp; 
	}
	else {
		SM_LOG(SM_CORE, SM_LOG_DEBUG, "Insufficient space on the tape, failed to allocate array");
		return NULL;
	}
}

int **sm_tape_allocate_2d_array(sm_tape_dcb *dcb, size_t rows, size_t cols) {
	void *tmp = (void *)SM_ALIGN(dcb->position, sizeof(int *));
	void *end = SM_OFFSET(void, dcb->position, int, 2);
	      end = SM_OFFSET(void, dcb->position, int *, rows);
	      end = SM_OFFSET(void, dcb->position, int, rows * cols);
	if(end < dcb->end) {
		dcb->position = tmp;
		sm_tape_write_int(dcb, rows);
		sm_tape_write_int(dcb, cols);
		tmp = dcb->position;
		for(int i = 0; i < rows; i++)
			((int **)(tmp))[i] = SM_OFFSET(int, tmp, int, (rows + i * cols));
		dcb->position = end;
		return (int **)tmp; 
	}
	else {
		SM_LOG(SM_CORE, SM_LOG_DEBUG, "Insufficient space on the tape, failed to allocate 2d array");
		return NULL;
	}
}

int sm_tape_write_string(sm_tape_dcb *dcb, const char *str) {
	void *end = SM_OFFSET(void, dcb->position, char, 2);
	if(end < dcb->end) {
		memcpy((char *)dcb->position, str, strlen(str));
		dcb->position = end;
		return EXIT_SUCCESS; 
	}
	else {
		SM_LOG(SM_CORE, SM_LOG_DEBUG, "Insufficient space on the tape, failed to write string");
		return EXIT_FAILURE;
	}
}

int sm_tape_read_string(sm_tape_dcb *dcb, char *str) {
	void *end = SM_OFFSET(void, dcb->position, char, strlen((char *)dcb->position) + 1);
	if(end < dcb->end) {
		memcpy((char *)dcb->position, str, strlen(str));
		dcb->position = end;
		return EXIT_SUCCESS; 
	}
	else {
		SM_LOG(SM_CORE, SM_LOG_DEBUG, "Failed to read unterminated or too long string");
		return EXIT_FAILURE;
	}
}

void * sm_tape_ff(sm_tape_dcb *dcb, size_t size) { 
	void *end = SM_OFFSET(void, dcb->position, char, size);
	if(end < dcb->end) {
		void *tmp = dcb->position;
		dcb->position = end;
		return tmp; 
	}
	else {
		SM_LOG(SM_CORE, SM_LOG_DEBUG, "Failed to FF tape - insufficient space");
		return NULL;
	}
}
