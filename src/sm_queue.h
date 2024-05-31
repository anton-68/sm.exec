/* SM.EXEC
   Queue
   anton.bondarenko@gmail.com */

#ifndef SM_QUEUE_H
#define SM_QUEUE_H

#include "sm_event.h"

/*
 0         1          2         3          4         5          6
 0123456789012345 6789012345678901 2345678901234567890123456789 0 123
+--------------------------------------------------------------------+ <-----
|                                HEAD                                |   | 
+----------------+----------------+----------------------------+-+---+   | simple 
|    0x7ffff1*   |       ID       |           SIZE             |S|###|   | queue 
+----------------+----------------+----------------------------+-+---+   | header
|                                TAIL                                |   |
+--------------------------------------------------------------------+ <-----
:                                                                    :   | o
+   -     -     -     -     -     -     -     -     -     -     -    +   | p
:                                                                    :   | t
+  -     -     -     -     -     -     -     -     -     -     -     +   | i
:                             [ LOCK ]                               :   | o
+ -     -     -     -     -     -     -     -     -     -     -     -+   | n
:                                                                    :   | a
+-     -     -     -     -     -     -     -     -     -     -     - +   | l
:                                                                    :   |   
+--------------------------------------------------------------------+   | e
:                                                                    :   | x
+   -     -     -     -     -     -     -     -     -     -     -    +   | t
:                                                                    :   | e 
+  -     -     -     -     -     -     -     -     -     -     -     +   | n
:                                                                    :   | s
+ -     -     -     -     -     -     -     -     -     -     -     -+   | i
:                             [ EMPTY ]                              :   | o
+-     -     -     -     -     -     -     -     -     -     -     - +   | n
:                                                                    :   |   
+     -     -     -     -     -     -     -     -     -     -     -  +   | 
:                                                                    :   |
+--------------------------------------------------------------------+ <-----

* Default = SM_QUEUE_TYPE_CODE (0x7ffff1) 

*/

typedef struct sm_queue {
	sm_event * head;	  //  8B
	uint16_t id_ext;	  //  2B
	uint16_t id;	      //  2B
	struct {              //  4B
		unsigned int size				 : 28;
		unsigned int synchronized		 :  1;
		unsigned int /* reserved */		 :  0; // 3 bits
	} ctl;	
    sm_event * tail;      //  8B
    //pthread_mutex_t lock; // 40B
    //pthread_cond_t empty; // 48B
} sm_queue;


/*****************************  
 *	Queue lifecycle methods  *
 *****************************/

sm_queue *sm_queue_create(size_t event_size, 
						  size_t num_of_events, 
						  bool synchronized,   
						  bool handle_flag,
						  bool hash_key_flag,
						  bool priority_flag);

void sm_queue_purge(sm_queue *q);

void sm_queue_free(sm_queue *q);


/**********************  
 *	Queue operations  *
 **********************/

int sm_queue_append(sm_queue *q1, sm_queue *q2);

size_t sm_queue_size(sm_queue *q);

sm_event * sm_queue_top(const sm_queue * q);

int sm_queue_enqueue(sm_event *e, sm_queue *q);

sm_event *sm_queue_dequeue(sm_queue *q);


#endif //SM_QUEUE_H
