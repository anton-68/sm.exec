/* SM.EXEC
   Event module
   (c) anton.bondarenko@gmail.com */

#ifndef SM_EVENT_H
#define SM_EVENT_H

#include <stdint.h>	
#include "sm_sys.h"
#include "sm_hash.h"
#include "sm_handle.h"
//#include "sm_queue.h"

/*
 0         1          2         3          4         5              6
 0123456789012345 6789012345678901 234567890123456789012345 6 7 8 9 0 1 23
+-------------------------------------------------------------------------+ <-----
|                                    NEXT                                 |   | simple
+----------------+----------------+------------------------+-+-+-+-+-+-+--+   | event 
|     0x7ffe*    |       ID       |           SIZE         |L|D|K|T|H|Q|##|   | header
+----------------+----------------+------------------------+-+-+-+-+-+-+--+ <-----
:                               [ KEY HASH ]                              :   | o e
+   -     -     -     -     -     -     -     -     -     -     -     -   +   | p x
:                              [ KEY STRING ]                             :   | t t
+-------------------------------------------------------------------------+   | i e
:                             [ PRIORITY[0] ]**                           :   | o n
+  -     -     -     -     -     -     -     -     -     -     -     -    +   | n s
:                             [ PRIORITY[1] ]**                           :   | a i
+-------------------------------------------------------------------------+   | l o
:                            [ HANDLE ADDRESS ]                           :   |   n
+-------------------------------------------------------------------------+   |   s
:                          [ HOME QUEUE ADDRESS ]                         :   |
+-------------------------------------------------------------------------+ <-----

* Default = SM_EVENT_TYPE_CODE (0x7ffe) 

** Timestamp fields also used for storing generalized priority values for operations
  of prioritizied queue

*/

struct sm_queue; // Forward decl.

typedef	struct sm_event	{
	struct sm_event *next;
	uint16_t id_ext;
	uint16_t id;
	struct {					
		unsigned int size			 	 : 24; // in 64-bit chunks
		unsigned int linked_flag		 :  1; // event has linked one 
		unsigned int disposable_flag	 :  1; // event is 'disposable'
		unsigned int key_flag		 	 :  1; // hash key included
		unsigned int priority_flag		 :  1; // timestamp/priority included
		unsigned int handle_flag		 :  1; // external handler address included
		unsigned int home_queue_flag	 :  1; // home queue address included
		unsigned int /* reserved */		 :  0; // 2 bits
	} ctl;
 // sm_hash_key key	
 // sm_event_priority priority[2];
 // sm_handle **handle;
 // struct sm_queue *home_queue; 	
} sm_event;


/*****************************  
 *	Event lifecycle methods  *
 *****************************/

sm_event *sm_event_create(size_t data_size, // in bytes
						    bool key_flag, 
						    bool priority_flag,
						    bool handle_flag,
			                bool home_queue_flag);

sm_event *sm_event_create_pool(size_t pool_size,
							   size_t data_size,
							     bool key_flag, 
						         bool priority_flag,
						         bool handle_flag,
			                     bool home_queue_flag);

sm_event *sm_event_duplicate(sm_event *e, struct sm_queue *q);

void sm_event_free(sm_event *e);    // ~sm_event(e);

void sm_event_purge(sm_event *e);   // data = '0'[data_size * 8]

void sm_event_recycle(sm_event *e); // enqueue in home queue


/********************** 
 *	Event ID methods  *
 **********************/

void sm_event_set_id(sm_event *e, size_t id); // Check range

size_t sm_event_id(sm_event *e);


/****************************** 
 *	Event validation methods  *
 ******************************/

bool sm_event_check_type(sm_event *e);


/****************************** 
 *  Event data block methods  *
 ******************************/

void *sm_event_data_ptr(sm_event *e);

size_t sm_event_data_size(sm_event *e);   // In bytes


/************************************* 
 *  Extended header field accessors  *
 *************************************/

sm_hash_key *sm_event_hash_key_ptr(sm_event *e);

sm_event_priority *sm_event_priority_ptr(sm_event *e);

sm_handle *sm_event_handle_ptr(sm_event *e);

struct sm_queue *sm_event_home_queue_ptr(sm_event *e);


#endif //SM_EVENT_H
