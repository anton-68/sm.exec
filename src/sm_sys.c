/* SM.EXEC
   Core functions
   anton.bondarenko@gmail.com */
   
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include "sm_sys.h"



    
     
/* thread id */

#ifdef __linux__
void get_tid_str(char *strID){
    sprintf(strID, "%i:%i", getpid(), (int)syscall(__NR_gettid));
}
#else
unsigned long gettid(){
    pthread_t ptid = pthread_self();
    unsigned long utid = 0;
    memcpy(&utid, &ptid, sizeof(utid)<sizeof(ptid)?sizeof(utid):sizeof(ptid));
    return utid;
}
void get_tid_str(char *strID){
    sprintf(strID, "%i:%lu", getpid(), (unsigned long)gettid());
}
#endif

#ifdef __linux__
unsigned long get_tid(){
    return (unsigned long)syscall(__NR_gettid);
}
#else
unsigned long get_tid(){
    pthread_t ptid = pthread_self();
    unsigned long utid = 0;
    memcpy(&utid, &ptid, sizeof(utid)<sizeof(ptid)?sizeof(utid):sizeof(ptid));
    return utid;
}
#endif