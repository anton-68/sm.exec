/* SM.EXEC
   Some system utilities and definintions
   (c) anton.bondarenko@gmail.com */

#ifndef SM_SYS_H
#define SM_SYS_H

#include <pthread.h>

/* event & state Ids */
typedef size_t SM_EVENT_ID;
typedef size_t SM_STATE_ID;

/* bool */
#if __STDC_VERSION__ >= 199901L
    #include <stdbool.h>
#else
    typedef int bool;
    enum { false, true };
#endif

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) >= (Y)) ? (X) : (Y))

/* JSON Parser 
   not defined == dynamic allocation */
//#define SM_FSM_NUM_OF_TOKENS 4096
#define SM_FSM_JSON_STRLEN 256

/* Multithreading */
#define TL_MUTEX_DEBUG

/* Mutex type */ 
#define TL_MUTEX_TYPE PTHREAD_MUTEX_DEFAULT
                   /* PTHREAD_MUTEX_ERRORCHECK */

/* Number of priority stages*/
#define SM_NUM_OF_PRIORITY_STAGES 2

/* DUMMY_PAYLOAD */
#define TL_DUMMY_PAYLOAD 0x012357BD
#define TL_DUMMY_PAYLOAD_SIZE sizeof TL_DUMMY_PAYLOAD

/* State Array hash key length */
#define SM_STATE_HASH_KEYLEN 256

/* FSM pretty print buffer */
#define OUTPUT_BUF_LEN (1024 * 32)

/* Thread ID */
unsigned long get_tid();
void get_tid_str(char *);

#endif //SM_SYS_H