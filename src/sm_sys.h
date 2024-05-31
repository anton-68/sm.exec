/* SM.EXEC
   Some system utilities and definintions
   (c) anton.bondarenko@gmail.com */

#ifndef SM_SYS_H
#define SM_SYS_H

#include <pthread.h> 
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

/* Program Name */
#ifndef SM_NAME
#ifdef SM_DEBUG
#define SM_NAME "SM.DEBUG"
#else
#define SM_NAME "SM.EXEC"
#endif
#endif

/* bool type */ 
#if __STDC_VERSION__ >= 199901L
	#include <stdbool.h>
#else
	typedef enum {false, true} bool;
#endif

/* Machine word defs */
#define SM_WORD (size_t)8
typedef ptrdiff_t sm_word_t;

/* Timestamp */
typedef struct sm_timestamp{
    time_t seconds;
    long microseconds;
    char timestring[64];
} sm_timestamp;
sm_timestamp sm_get_timestamp();

/* Maximal event id */
#define SM_EVENT_ID_MAX UINT16_MAX

/* Maximal event data size */
#define SM_EVENT_MAX_DATA_SIZE (1<<24 - 1)

/* Dummy event payload for queues*/
#define SM_DUMMY_PAYLOAD 0x012357BD
#define SM_DUMMY_PAYLOAD_SIZE sizeof SM_DUMMY_PAYLOAD

/* Numner of event priority steges */
#define SM_NUM_OF_PRIORITY_STAGES 2

/* Event priority type */
typedef long int sm_event_priority;

/* Mutex type */
#ifndef SM_DEBUG
#define SM_MUTEX_TYPE PTHREAD_MUTEX_DEFAULT
#else
#define SM_MUTEX_TYPE PTHREAD_MUTEX_ERRORCHECK
#endif

/* Mutex operations */
#define SM_LOCK_NAME lock
#define SM_COND_NAME empty
#define SM_ATTR_NAME attr
#define SM_LOCK_INIT(q, _free) \
	if((q)->ctl.synchronized){ \
    	pthread_mutexattr_t SM_ATTR_NAME; \
    	if(pthread_mutexattr_init(&SM_ATTR_NAME) != EXIT_SUCCESS) { \
        	SM_LOG(SM_CORE, SM_LOG_ERR, "Failed to create queue mutex attribute"); \
        	_free(q); \
        	return NULL; \
    	} \
    	if(pthread_mutexattr_settype(&SM_ATTR_NAME, SM_MUTEX_TYPE) != EXIT_SUCCESS){ \
        	SM_LOG(SM_CORE, SM_LOG_ERR, "Failed to set queue mutex attribute type"); \
        	_free(q); \
        	return NULL; \
    	} \
    	if(pthread_mutex_init(&((q)->SM_LOCK_NAME), &SM_ATTR_NAME) != EXIT_SUCCESS) { \
        	SM_LOG(SM_CORE, SM_LOG_ERR, "Failed to initialize queue mutex"); \
        	_free(q); \
        	return NULL; \
    	} \
    	if(pthread_cond_init(&((q)->SM_COND_NAME),NULL) != EXIT_SUCCESS) { \
        	SM_LOG(SM_CORE, SM_LOG_ERR, "Failed to initialize queue conditioinal"); \
        	pthread_cond_signal(&(q)->SM_COND_NAME); \
        	_free(q); \
        	return NULL; \
    	} \
	}
#define SM_LOCK_DESTROY(q) \
	if((q)->ctl.synchronized){ \
    	pthread_mutex_destroy(&(q)->SM_LOCK_NAME); \
    	pthread_cond_destroy(&(q)->SM_COND_NAME); \
	}
#define SM_LOCK(q) \
	if((q)->ctl.synchronized) { \
    	if(pthread_mutex_lock(&((q)->SM_LOCK_NAME)) != EXIT_SUCCESS) { \
			SM_LOG(SM_CORE, SM_LOG_ERR, "Failed to lock mutex"); \
        	return EXIT_FAILURE; \
   		} \
	}
#define SM_SIGNAL_UNLOCK(q) \
	if((q)->ctl.synchronized){ \
    	if(pthread_cond_signal(&((q)->SM_COND_NAME)) != EXIT_SUCCESS) { \
			SM_LOG(SM_CORE, SM_LOG_ERR, "Failed to signal conditional"); \
        	return EXIT_FAILURE; \
    	} \
    	if(pthread_mutex_unlock(&((q)->SM_LOCK_NAME)) != EXIT_SUCCESS) { \
			SM_LOG(SM_CORE, SM_LOG_ERR, "Failed to unlock mutex"); \
        	return EXIT_FAILURE; \
    	} \
	}
#define SM_LOCK_WAIT(q, _expr) \
	if((q)->ctl.synchronized){ \
    	int __tl_result = pthread_mutex_lock(&((q)->SM_LOCK_NAME)); \
    	if(__tl_result != EXIT_SUCCESS) { \
        	SM_LOG(SM_CORE, SM_LOG_ERR, "Failed to lock mutex"); \
        	return NULL; \
    	} \
    	while((_expr) == NULL) { \
        	__tl_result = pthread_cond_wait(&((q)->SM_COND_NAME), &((q)->lock)); \
        	if (__tl_result != EXIT_SUCCESS) { \
            	SM_LOG(SM_CORE, SM_LOG_ERR, "Failed waiting conditional"); \
            	return NULL; \
        	} \
    	} \
    	__tl_result = pthread_mutex_unlock(&((q)->SM_LOCK_NAME)); \
    	if(__tl_result != EXIT_SUCCESS) { \
        	SM_LOG(SM_CORE, SM_LOG_ERR, "Failed to unlock mutex"); \
        	return NULL; \
    	} \
	} \
	else { \
		_expr; \
	}

/* MIN & MAX */
#define SM_MIN(X, Y) (((X) < (Y)) ? (X) : (Y))  //--??
#define SM_MAX(X, Y) (((X) >= (Y)) ? (X) : (Y)) //--??

/* CEILING & FLOOR */
#define SM_CEILING(X, Y) ((X) + (Y) - 1) / (Y)
#define SM_FLOOR(X, Y) ((X) + ((X) + (Y)) / 2) / (Y)

/* Alignment */
#define SM_ALIGN(K, L) (SM_CEILING((size_t)(char *)(K), (L)) * (L))
#define SM_OFFSET(T1, A, T2, O) (T1*)((T2*)(A) + (ptrdiff_t)(O))
#define SM_WOFFSET(T1, A, O) SM_OFFSET(T1, A, sm_word_t, O) 

/* dlopen() flag */
#define SM_APP_RTLD_FLAG RTLD_NOW | RTLD_GLOBAL

/* Hex object type identifying sequences */
#define SM_ARRAY_TYPE_CODE  0x7ff0
#define SM_QUEUE_TYPE_CODE  0x7ff1
#define SM_QUEUE2_TYPE_CODE 0x7ff2
#define SM_PQUEUE_TYPE_CODE 0x7ff3
#define SM_FSM_TYPE_CODE    0x7fff
#define SM_EVENT_TYPE_CODE  0x7ffe

/* JSON Parser 
   not defined == dynamic allocation */
//#define SM_FSM_NUM_OF_TOKENS 4096
#define SM_FSM_JSON_STRLEN 256

/* Multithreading */
#define TL_MUTEX_DEBUG

/* State Array hash key length */
#define SM_STATE_HASH_KEYLEN 256

/* FSM pretty print buffer */
#define SM_OUTPUT_BUF_LEN (1024 * 32)
char sm_buffer[SM_OUTPUT_BUF_LEN];

/* Thread ID */
unsigned long get_tid();
void get_tid_str(char *);

/* Allocation boundary */
#define SM_MEMORY_WORD 8

/* SM ID */
//typedef uint32_t SM_ID;

// public methods 
void sm_id_to_ipstr(uint32_t id, char* const ip);
uint32_t sm_ipstr_to_id(const char* const ip);

/* SM Default exectutor descriptor data size */
#define SM_EXEC_DATA_SIZE 4096

/* SM Default thread-worker descriptor data size */
#define SM_TX_DATA_SIZE 4096

/* SM Default state descriptor data size */
#define SM_STATE_DATA_SIZE 4096

/* SM Default event descriptor data size */
#define SM_EVENT_DATA_SIZE 4096

/* SM Simple memory manager usage flag 
   Restrict data block sizes for events, states and exec descriptor */
#ifndef SM_MEMORY_MANAGER
#define SM_MEMORY_MANAGER false
#endif

#define SM_SYSLOG_STRING_LEN 4096

#define SM_SPINLOCK_NS 1000 // 1usec

#endif //SM_SYS_H