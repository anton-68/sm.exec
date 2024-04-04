/* SM.EXEC
   Some system utilities and definintions
   anton.bondarenko@gmail.com */


// #include <stdint.h>
// #include <stdlib.h>

#include <stdio.h>			// sprintf()
#include <sys/types.h>		// getpid()
#include <unistd.h>			// getpid(), syscall()
#include <string.h>			// memcpy()
#include <sys/syscall.h>	// syscall()
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