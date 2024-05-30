/* DT.EXEC
   Autogenerate - don't change!
   (c) anton.bondarenko@gmail.com */

#include "dt_sys.h"
#include "sm_fsm_transition_dt.h"

bool compare_sm_fsm_transition_dt_state(size_t a, size_t b) {
	return int_eq((int)a, (int)b);
}

bool compare_sm_fsm_transition_dt_event(size_t a, size_t b) {
	return int_eq((int)a, (int)b);
}

sm_fsm_transition_dt_output *find(const sm_fsm_transition_dt_input *i, 
								 const sm_fsm_tramsition_dt *t,
								 size_t fs_tab_size) {
	sm_fsm_transition_dt_output *o;
	for (int i = 0; i < fs_tab_size; i++) {
		if(compare_sm_fsm_transition_dt_state(
			
	}
	
	
	
	return o;
}