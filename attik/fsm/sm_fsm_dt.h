/* DT.EXEC
   Autogenerate - don't change!
   (c) anton.bondarenko@gmail.com */

// Header fot sm_fsm decision table

#include "dt_sys.h"
#include "sm_app.h"

typedef struct sm_fsm_dt_input {
	size_t state;
	size_t event;
} sm_fsm_dt_input;

typedef struct sm_fsm_dt_output {
	size_t state;
	sm_app action;
} sm_fsm_dt_output;

typedef struct sm_fsm_dt_rule {
	sm_fsm_dt_input input;
	sm_fsm_dt_output output;
} sm_fsm_dt_rule;
	
typedef struct sm_fsm_dt_table {
	size_t size;
	sm_fsm_dt_rule** rules;
	// ...
} sm_fsm_dt_table;

// parse json
// parse csv
// init table

bool check_sm_fsm_dt_state(size_t a, size_t b);
bool check_sm_fsm_dt_event(size_t a, size_t b);

sm_fsm_dt_output *lookup(const sm_fsm_dt_table *t, 
						 const sm_fsm_dt_input *i);