#include <stdio.h>
#include <stdint.h>

int main() {
	struct {
		unsigned int id					 : 16;
		unsigned int t_flag		 		 :  1;
		unsigned int d_flag	 			 :  1;
		unsigned int v_flag	 			 :  1;
		unsigned int /* reserved */		 :  0; // 2 bits
	} ctl;
	ctl.t_flag = 1;
	ctl.d_flag = 1;
	ctl.id = - ctl.d_flag - ctl.t_flag;
	printf("sizeof(bitmap) = %ld\n", sizeof(ctl));
	printf("sizeof(bit + bit) = %ld\n", sizeof(ctl.d_flag + ctl.t_flag));
	printf("(uint16_t)(-(bit)1 + -(bit)1) = %d\n", ctl.id);
	printf("-(bit)1 + -(bit)1 = %d\n", - ctl.d_flag - ctl.t_flag);
	ctl.v_flag = ctl.d_flag + ctl.t_flag;
	printf("(bit)v = (bit)1 + (bit)1 = %d\n", ctl.v_flag); 
	
	printf("sizeof(event header) = %ld\n", sizeof(struct sm_event	{
		struct sm_event *next;
		uint32_t data_size;
		struct {
			unsigned int id					 : 16;
			unsigned int data_offset		 :  8; // in x64 words
			unsigned int tailed_flag		 :  1;
			unsigned int disposable_flag	 :  1;
			unsigned int pool_flag		 	 :  1;
			unsigned int handle_flag		 :  1; // managed by 'handler'
			unsigned int hash_key_flag		 :  1;
			unsigned int priority_flag		 :  1;
			unsigned int /* reserved */		 :  0; // 2 bits
		} ctl;
	}));
	return 0;
}