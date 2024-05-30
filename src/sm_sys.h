/* SM.EXEC
   Some system utilities and definintions
   (c) anton.bondarenko@gmail.com */

#ifndef SM_SYS_H
#define SM_SYS_H

#include <pthread.h> 
#include <stddef.h>
#include <stdint.h>
#include <time.h>

/* Program Name */
#ifndef SM_NAME
#ifdef SM_DEBUG
#define SM_NAME "SM.DEBUG"
#else
#define SM_NAME "SM.EXEC"
#endif
#endif

/* bool */ 
#if __STDC_VERSION__ >= 199901L
	#include <stdbool.h>
#else
	typedef enum { false, true} bool;
#endif

/* Machine word defs */
#define SM_WORD 8
typedef ptrdiff_t sm_word_t;

/* Timestamp */
typedef struct sm_timestamp{
    time_t seconds;
    long microseconds;
    char timestring[32];
} sm_timestamp;
sm_timestamp sm_get_timestamp();














/* MIN & MAX */
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))  //--??
#define MAX(X, Y) (((X) >= (Y)) ? (X) : (Y)) //--??



/* JSON Parser 
   not defined == dynamic allocation */
//#define SM_FSM_NUM_OF_TOKENS 4096
#define SM_FSM_JSON_STRLEN 256

/* Multithreading */
#define TL_MUTEX_DEBUG

/* Mutex type */ 
#define TL_MUTEX_TYPE PTHREAD_MUTEX_DEFAULT
                   /* PTHREAD_MUTEX_ERRORCHECK */

/* Dammy payload */
#define TL_DUMMY_PAYLOAD 0x012357BD
#define TL_DUMMY_PAYLOAD_SIZE sizeof TL_DUMMY_PAYLOAD

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