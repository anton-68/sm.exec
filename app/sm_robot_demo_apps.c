/* SM.EXEC 
   Test applications
   anton.bondarenko@gmail.com */

#include <time.h>
#include <limits.h>
#include "../src/sm_event.h"
#include "../src/sm_queue.h"
#include "../src/sm_state.h"
#include "../src/sm_array.h"
#include "../src/sm_memory.h"
#include "../src/sm_exec.h"

#include "sm_robot_demo_apps.h"

// Traffic SM7 (Golly)

// Manhattan
static size_t manhattan(int x1, int y1, int x2, int y2) {
	return SM_ABS(x1 - x2) + SM_ABS(y1 - y2);
}

// Selector
int trsim_check_neighbours(sm_event *e, sm_state *s) {

#ifdef SM_DEBUG
	REPORT(EVENT, "trsim_check_neighbours()");
#endif	
	
	sm_chunk *e_ch = sm_chunk_find(e->data, sm_ipstr_to_id("10.255.255.130"));
	if(e_ch == NULL) {
		REPORT(ERROR, "trsim_check_neighbours()");
		e->id = SM_TRSIM_ERROR;
		return 0;
	}

	void * e_cursor = e_ch->data;
	int X, Y, heading, goalX, goalY;
	e_cursor = sm_memory_read_int(e_ch, &e_cursor, &X);
	e_cursor = sm_memory_read_int(e_ch, &e_cursor, &Y);
	e_cursor = sm_memory_read_int(e_ch, &e_cursor, &heading);
	e_cursor = sm_memory_skip(e_ch, e_cursor, 8 * sizeof(int), sizeof(int)); // neighbourhood
	e_cursor = sm_memory_read_int(e_ch, &e_cursor, &goalX);
	e_cursor = sm_memory_read_int(e_ch, &e_cursor, &goalY);
	
	sm_chunk *x_ch = sm_chunk_find(s->exec->data, sm_ipstr_to_id("10.255.255.128"));
	if(x_ch == NULL) {
		REPORT(ERROR, "trsim_check_neighbours()");
		e->id = SM_TRSIM_ERROR;
		return 0;
	}
	void * x_cursor = x_ch->data;
	int rows, cols;
	x_cursor = sm_memory_read_int(x_ch, &x_cursor, &rows);
	x_cursor = sm_memory_read_int(x_ch, &x_cursor, &cols);	
	
	if (manhattan(X, Y, goalX, goalY) == 1) {
		if(goalY - Y == 1)
			heading = 1; //N
		else if(goalX - X == 1)
			heading = 2; //E
		else if(Y - goalY == 1)
			heading = 3; //S
		else if(X - goalX == 1)
			heading = 2; //W
		e->id = 1;
		e_cursor = e_ch->data; // rewind
		e_cursor = sm_memory_skip(e_ch, e_cursor, sizeof(int) * 2, sizeof(int));
		e_cursor = sm_memory_write_int(e_ch, &e_cursor, heading);
		return 0;
	} 
	if(manhattan(X, Y, goalX, goalY) == 0) {
		if(X == cols/2 - 2) { // right edge -> step down 
		   	goalX = X;
			goalY = Y + 1;
		}
		else if(X == cols%2 - cols/2 + 1) { // left edge -> step up 
		   	goalX = X;
			goalY = Y - 1;
		}
		else if(Y == rows/2 - 2) { // bottom edge -> step left 
		   	goalX = X - 1;
			goalY = Y;
		}
		else if(Y == rows%2 - rows/2 + 1) { // top edge -> step right 
		   	goalX = X + 1;
			goalY = Y;
		}
		e_cursor = e_ch->data; // rewind
		e_cursor = sm_memory_skip(e_ch, e_cursor, sizeof(int) * 2, sizeof(int));
		e_cursor = sm_memory_write_int(e_ch, &e_cursor, heading);
		e_cursor = sm_memory_skip(e_ch, e_cursor, 8 * sizeof(int), sizeof(int));
		e_cursor = sm_memory_write_int(e_ch, &e_cursor, goalX);
		e_cursor = sm_memory_write_int(e_ch, &e_cursor, goalY);
		e->id = 1;
		return 0;
	}
	e->id = 2;
	return 0;
}

// Find state and push
int trsim_switch_fsm(sm_event *e, sm_state *s){

#ifdef SM_DEBUG
	REPORT(EVENT, "trsim_switch_fsm()");
#endif	

	sm_chunk *e_ch = sm_chunk_find(e->data, sm_ipstr_to_id("10.255.255.130"));
	if(e_ch == NULL) {
		REPORT(ERROR, "trsim_switch_fsm()");
		e->id = SM_TRSIM_ERROR;
		return 0;
	}
	void * e_cursor = e_ch->data;
	int goalX, goalY;
	e_cursor = sm_memory_skip(e_ch, e_cursor, 11 * sizeof(int), sizeof(int)); // neighbourhood
	e_cursor = sm_memory_read_int(e_ch, &e_cursor, &goalX);
	e_cursor = sm_memory_read_int(e_ch, &e_cursor, &goalY);
	
	sm_chunk *x_ch = sm_chunk_find(s->exec->data, sm_ipstr_to_id("10.255.255.128"));
	if(x_ch == NULL) {
		REPORT(ERROR, "trsim_switch_fsm()");
		e->id = SM_TRSIM_ERROR;
		return 0;
	}
	void * x_cursor = x_ch->data;
	int rows, cols;
	x_cursor = sm_memory_read_int(x_ch, &x_cursor, &rows);
	x_cursor = sm_memory_read_int(x_ch, &x_cursor, &cols);

	int key = (SM_MAX(cols, rows) * goalX + goalY);
	sm_array ** states = (sm_array **)sm_directory_get_ref(s->exec->dir, "states");
	sm_state *planning_state = sm_array_get_state(*states, (void *)&key, sizeof(int));
	if(planning_state == NULL) {
		REPORT(ERROR, "trsim_switch_fsm()");
		e->id = SM_TRSIM_ERROR;
		return 0;
	}
	sm_tx_push_state(s->tx, planning_state);
	s->tx->state = planning_state;
	return 0;
}

// Brushfire algorithm
int trsim_brushfire(sm_event *e, sm_state *s) {
	
#ifdef SM_DEBUG
	REPORT(EVENT, "trsim_brushfire()");
#endif		
	
	// get brushfire queues
	sm_queue *bf_pool = *(sm_directory_get_ref(s->exec->dir, "bf_pool"));
	sm_queue *bf_queue = *(sm_directory_get_ref(s->exec->dir, "bf_queue"));
	
	// get map
	sm_chunk *s_ch = sm_chunk_find(s->data, sm_ipstr_to_id("10.255.255.129"));
	if(s_ch == NULL) {
		REPORT(ERROR, "trsim_brushfire()");
		e->id = SM_TRSIM_ERROR;
		return 0;
	}
	void * s_cursor = s_ch->data;
	int rows, cols, goalX, goalY;
	s_cursor = sm_memory_read_int(s_ch, &s_cursor, &rows);
	s_cursor = sm_memory_read_int(s_ch, &s_cursor, &cols);
	int ** map = (int **)s_cursor;
	s_cursor = sm_memory_skip(s_ch, s_cursor, rows * sizeof(int *), sizeof(int *));
	s_cursor = sm_memory_skip(s_ch, s_cursor, rows * cols * sizeof(int), sizeof(int));	
	s_cursor = sm_memory_read_int(s_ch, &s_cursor, &goalX);
	s_cursor = sm_memory_read_int(s_ch, &s_cursor, &goalY);

	// go!
	sm_event *bf_e, *bf_e1;
	bf_e = sm_queue_dequeue(bf_pool);
	SM_X(bf_e) = goalX;
	SM_Y(bf_e) = goalY;
	sm_queue_enqueue(bf_e, bf_queue);
	while(sm_queue_size > 0) {
		bf_e = sm_queue_dequeue(bf_queue);
		if(map[SM_X(bf_e)][SM_Y(bf_e) + 1] < INT_MAX - 1 &&		// North
		   map[SM_X(bf_e)][SM_Y(bf_e) + 1] >  map[SM_X(bf_e)][SM_Y(bf_e)] + 1) {
			map[SM_X(bf_e)][SM_Y(bf_e) + 1] = map[SM_X(bf_e)][SM_Y(bf_e)] + 1;
			bf_e1 = sm_queue_dequeue(bf_pool);
			SM_X(bf_e1) = SM_X(bf_e);
			SM_Y(bf_e1) = SM_Y(bf_e) + 1;
			sm_queue_enqueue(bf_e1, bf_queue);
		}
		if(map[SM_X(bf_e) + 1][SM_Y(bf_e)] < INT_MAX - 1 &&		// East
		   map[SM_X(bf_e) + 1][SM_Y(bf_e)] > map[SM_X(bf_e)][SM_Y(bf_e)] + 1) {
			map[SM_X(bf_e) + 1][SM_Y(bf_e)] = map[SM_X(bf_e)][SM_Y(bf_e)] + 1;
			bf_e1 = sm_queue_dequeue(bf_pool);
			SM_X(bf_e1) = SM_X(bf_e) + 1;
			SM_Y(bf_e1) = SM_Y(bf_e);
			sm_queue_enqueue(bf_e1, bf_queue);
		}
		if(map[SM_X(bf_e)][SM_Y(bf_e) - 1] < INT_MAX - 1 &&		// South
		   map[SM_X(bf_e)][SM_Y(bf_e) - 1] > map[SM_X(bf_e)][SM_Y(bf_e)] + 1) {
			map[SM_X(bf_e)][SM_Y(bf_e) - 1] = map[SM_X(bf_e)][SM_Y(bf_e)] + 1;
			bf_e1 = sm_queue_dequeue(bf_pool);
			SM_X(bf_e1) = SM_X(bf_e);
			SM_Y(bf_e1) = SM_Y(bf_e) - 1;
			sm_queue_enqueue(bf_e1, bf_queue);
		}
		if(map[SM_X(bf_e) - 1][SM_Y(bf_e)] < INT_MAX - 1 &&		// East
		   map[SM_X(bf_e) - 1][SM_Y(bf_e)] > map[SM_X(e)][SM_Y(e)] + 1) {
			map[SM_X(bf_e) - 1][SM_Y(bf_e)] = map[SM_X(bf_e)][SM_Y(bf_e)] + 1;
			bf_e1 = sm_queue_dequeue(bf_pool);
			SM_X(bf_e1) = SM_X(bf_e) - 1;
			SM_Y(bf_e1) = SM_Y(bf_e);
			sm_queue_enqueue(bf_e1, bf_queue);
		}
		sm_queue_enqueue(bf_e, bf_pool);
	}
	return 0;
}


// Planner
int trsim_plan(sm_event *e, sm_state *s) {

#ifdef SM_DEBUG
	REPORT(EVENT, "trsim_plan()");
#endif		
	
	int cols, rows;
	void * position;
	int **x_map, **s_map;
	
	sm_chunk *s_ch = sm_chunk_find(s->data, sm_ipstr_to_id("10.255.255.129"));
	if(s_ch == NULL) {
		REPORT(ERROR, "trsim_plan()");
		e->id = SM_TRSIM_ERROR;
		return 0;
	}
	void * s_cursor = s_ch->data;
	
	sm_chunk *e_ch = sm_chunk_find(e->data, sm_ipstr_to_id("10.255.255.130"));
	if(e_ch == NULL) {
		REPORT(ERROR, "trsim_plan()");
		e->id = SM_TRSIM_ERROR;
		return 0;
	}
	void * e_cursor = e_ch->data;
	
	if (*(int *)s_cursor == 0) { // Check if s->map is not active
		sm_chunk *x_ch = sm_chunk_find(s->exec->data, sm_ipstr_to_id("10.255.255.128"));
		if(x_ch == NULL) {
			REPORT(ERROR, "trsim_plan()");
			e->id = SM_TRSIM_ERROR;
			return 0;
		}
		void * x_cursor = x_ch->data;
		
		//copy map to the state datablock
		x_cursor = sm_memory_read_int(x_ch, &x_cursor, &rows);
		x_cursor = sm_memory_read_int(x_ch, &x_cursor, &cols);
		s_cursor = sm_memory_write_int(s_ch, &s_cursor, rows);
		s_cursor = sm_memory_write_int(s_ch, &s_cursor, cols);
		x_map = (int **)x_cursor;
		position = s_cursor; 
		s_cursor = sm_memory_allocate_2d_int_array(s_ch, &position, rows, cols);
		s_map = (int **)position;
		x_cursor = sm_memory_skip(x_ch, &x_cursor, rows * sizeof(int *), sizeof(int *));
		x_cursor = sm_memory_skip(x_ch, &x_cursor, rows * cols * sizeof(int), sizeof(int));
		for(int i = 0; i < rows; i++)
			for(int j = 0; j < cols; j++)
				if(x_map[i][j] == 0)
					s_map[i][j] = 0;
				else
					s_map[i][j] = INT_MAX - 1;
		x_cursor = sm_memory_skip(x_ch, &x_cursor, rows * sizeof(int *), sizeof(int *));
		x_cursor = sm_memory_skip(x_ch, &x_cursor, rows * cols * sizeof(int), sizeof(int));
		
		// Set state.goal = e.goal
		int goalX, goalY;
		e_cursor = sm_memory_skip(e_ch, e_cursor, sizeof(int) * 11, sizeof(int *));
		e_cursor = sm_memory_read_int(e_ch, &e_cursor, &goalX);
		e_cursor = sm_memory_read_int(e_ch, &e_cursor, &goalY);
		s_cursor = sm_memory_write_int(s_ch, s_cursor, goalX);
		s_cursor = sm_memory_write_int(s_ch, s_cursor, goalY);
		
		// Rewind
		s_cursor = s_ch->data;
		e_cursor = e_ch->data;
		
		// Call brushfire
		trsim_brushfire(e, s);
	}
	
	// Rewind
	s_cursor = s_ch->data;
	e_cursor = e_ch->data;
	
	// Determine heading
	int heading = 5;
	int X, Y;
	e_cursor = sm_memory_read_int(e_ch, &e_cursor, &X);
	e_cursor = sm_memory_read_int(e_ch, &e_cursor, &Y);
	s_cursor = sm_memory_read_int(s_ch, &s_cursor, &rows);
	s_cursor = sm_memory_read_int(s_ch, &s_cursor, &cols);
	s_map = (int **)s_cursor;
	
	int c = s_map[X][Y];
	if(s_map[X][Y-1] + 1 < c){
		heading = 1;
		c = s_map[X][Y-1] + 1;
	}
	if(s_map[X+1][Y] + 1 < c){
		heading = 2;
		c = s_map[X+1][Y] + 1;
	}
	if(s_map[X][Y+1] + 1 < c){
		heading = 3;
		c = s_map[X][Y+1] + 1;
	}
	if(s_map[X-1][Y] + 1 < c){
		heading = 4;
		c = s_map[X-1][Y] + 1;
	}
	s_map[X][Y] = heading;
	// Set event id and return
	e->id = 1;
	return trsim_pop_fsm(e, s);
}

// Pop back
int trsim_pop_fsm(sm_event *e, sm_state *s){
	
#ifdef SM_DEBUG
	REPORT(EVENT, "trsim_pop_fsm()");
#endif			
	
	sm_tx_pop_state(s->tx);
	return 0;
}

// No application
int trsim_noap(sm_event *e, sm_state *s) {return 0;}

