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

/* JSON Parser 
   not defined == dynamic allocation */
//#define SM_FSM_NUM_OF_TOKENS 4096
#define SM_FSM_JSON_STRLEN 256

/* multithreading */
#define TL_MUTEX_DEBUG

/* Mutex type */ 
#define TL_MUTEX_TYPE PTHREAD_MUTEX_DEFAULT
                   /* PTHREAD_MUTEX_ERRORCHECK */

/* DUMMY_PAYLOAD */
#define TL_DUMMY_PAYLOAD 0x012357BD
#define TL_DUMMY_PAYLOAD_SIZE sizeof TL_DUMMY_PAYLOAD

/* Thread ID */
unsigned long get_tid();
void get_tid_str(char *);

#define SM_RTLD_FLAG RTLD_NOW | RTLD_GLOBAL

#endif //SM_SYS_H