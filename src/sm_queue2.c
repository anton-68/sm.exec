/* SM.EXEC
   2-priority queue
   anton.bondarenko@gmail.com */

#include <stdlib.h>			// malloc-free

#include "sm_queue2.h"

/* sm_queue2 */

// Private methods
//sm_event *sm_dequeue2_low(sm_queue2 *q);
//sm_event *sm_dequeue2_high(sm_queue2 *q);

sm_queue2 *sm_queue2_create(/*size_t event_size, unsigned num_of_events*/) {
    sm_queue2 *q;
    if((q = malloc(sizeof(sm_queue2))) == NULL) {
        REPORT(ERROR, "malloc()");
        return NULL;
    }    
    sm_event * e;
    if((e = sm_event_create(TL_DUMMY_PAYLOAD_SIZE)) == NULL) {
        REPORT(ERROR, "sm_event_create()");
        free(q);
        return NULL;
    }
    *((unsigned*)(e->data)) = TL_DUMMY_PAYLOAD;
	e->home = NULL;
    q->h0 = q->t0 = e;
    if((e = sm_event_create(TL_DUMMY_PAYLOAD_SIZE)) == NULL) {
        REPORT(ERROR, "sm_event_create()");
        free(q);
        return NULL;
    }
    *((unsigned*)(e->data)) = TL_DUMMY_PAYLOAD;
	e->home = NULL;
    q->h1 = q->t1 = e;
	/*
	int i;
    for(i = 0; i < num_of_events; i++) {
        if((e = sm_event_create(TL_DUMMY_PAYLOAD_SIZE)) == NULL) {
            REPORT(ERROR, "event_create()");
            sm_queue2_free(q);
            return NULL;
        }
        sm_enqueue2(e, q);
    }
	*/
    pthread_mutexattr_t attr;
    if(pthread_mutexattr_init(&attr) != EXIT_SUCCESS) {
        REPORT(ERROR, "pthread_mutexattr_init()");
        sm_queue2_free(q);
        return NULL;
    }
    if(pthread_mutexattr_settype(&attr, TL_MUTEX_TYPE) != EXIT_SUCCESS){
        REPORT(ERROR, "pthread_mutexattr_settype()");
        sm_queue2_free(q);
        return NULL;
    }    
    if(pthread_mutex_init(&(q->queue0_lock),&attr) != EXIT_SUCCESS) {
        REPORT(ERROR, "pthread_mutex_init()");
        sm_queue2_free(q);
        return NULL;
    }
    if(pthread_mutex_init(&(q->queue1_lock),&attr) != EXIT_SUCCESS) {
        REPORT(ERROR, "pthread_mutex_init()");
        sm_queue2_free(q);
        return NULL;
    }    
    if(pthread_cond_init(&(q->empty),NULL) != EXIT_SUCCESS) {
        REPORT(ERROR, "pthread_cond_init()");
        pthread_cond_signal(&q->empty);
        sm_queue2_free(q);
        return NULL;
    }
    return q;
}

void sm_queue2_free(sm_queue2 *q) {
    sm_event * tmp; 
    sm_event * e = q->h0;
    while(e != NULL) {
        tmp = e->next;
        sm_event_free(e);
        e = tmp;
    }
    e = q->h1;
    while(e != NULL) {
        tmp = e->next;
        sm_event_free(e);
        e = tmp;
    }
    pthread_mutex_destroy(&q->queue0_lock);
    pthread_mutex_destroy(&q->queue1_lock);
    pthread_cond_destroy(&q->empty);
    free(q);
}

bool sm_queue2_is_empty(sm_queue2 * q) {
    return (q->h0->next == NULL && q->h1->next == NULL);
}

sm_event * sm_queue2_get(const sm_queue2 * q) {
    return q->h1->next;
}

sm_event * sm_queue2_get_high(const sm_queue2 * q) {
    return q->h0->next;
}

void sm_enqueue2(sm_event * e, sm_queue2 * q) {
    q->t1->next = e;
    q->t1 = e;
    q->t1->next = NULL;
}

void sm_enqueue2_high(sm_event * e, sm_queue2 * q) {
    q->t0->next = e;
    q->t0 = e;
    q->t0->next = NULL;
}

int sm_lock_enqueue2(sm_event * e, sm_queue2 * q){
    if(pthread_mutex_lock(&(q->queue1_lock)) != EXIT_SUCCESS) {
        REPORT(ERROR, "pthread_mutex_lock()");
        return EXIT_FAILURE;
    }
    sm_enqueue2(e, q);
    if(pthread_cond_signal(&(q->empty)) != EXIT_SUCCESS) {
        REPORT(ERROR, "pthread_cond_signal()");
        return EXIT_FAILURE;
    }
    if(pthread_mutex_unlock(&(q->queue1_lock)) != EXIT_SUCCESS) {
        REPORT(ERROR, "pthread_mutex_unlock()");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int sm_lock_enqueue2_high(sm_event * e, sm_queue2 * q){
    if(pthread_mutex_lock(&(q->queue0_lock)) != EXIT_SUCCESS) {
        REPORT(ERROR, "pthread_mutex_lock()");
        return EXIT_FAILURE;
    }
    sm_enqueue2_high(e, q);
    if(pthread_cond_signal(&(q->empty)) != EXIT_SUCCESS) {
        REPORT(ERROR, "pthread_cond_signal()");
        return EXIT_FAILURE;
    }
    if(pthread_mutex_unlock(&(q->queue0_lock)) != EXIT_SUCCESS) {
        REPORT(ERROR, "pthread_mutex_unlock()");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

static sm_event * sm_dequeue2_low(sm_queue2 * q) {
    sm_event * e = q->h1->next;
    if(e != NULL) {
        q->h1->next = q->h1->next->next;
        if(e->next == NULL)
            q->t1 = q->h1;
        else
            e->next = NULL;
    }
    return e;
}

static sm_event * sm_dequeue2_high(sm_queue2 * q) {
    sm_event * e = q->h0->next;
    if(e != NULL) {
        q->h0->next = q->h0->next->next;
        if(e->next == NULL)
            q->t0 = q->h0;
        else
            e->next = NULL;
    }
    return e;
}

sm_event * sm_dequeue2(sm_queue2 * q) {
    sm_event * e;
    if(q->h0->next != NULL)
        e = sm_dequeue2_high(q);
    else 
        e = sm_dequeue2_low(q);
    return e;
}    

sm_event * sm_lock_dequeue2(sm_queue2 * q) {
    sm_event * e;
    if(q->h0->next != NULL)
        if(q->h0->next->next != NULL)
            e = sm_dequeue2_high(q);
        else {
            if(pthread_mutex_lock(&(q->queue0_lock)) != EXIT_SUCCESS) {
                REPORT(ERROR, "pthread_mutex_lock()");
                return NULL;
            }
            e = sm_dequeue2_high(q);
            if(pthread_mutex_unlock(&(q->queue0_lock)) != EXIT_SUCCESS) {
                REPORT(ERROR, "pthread_mutex_unlock()");
                sm_enqueue2_high(e, q);
                e = NULL;
            }
        }
    else
        if(q->h1->next != NULL && q->h1->next->next != NULL)
            e = sm_dequeue2_low(q);
        else {
            if(pthread_mutex_lock(&(q->queue1_lock)) != EXIT_SUCCESS) {
                REPORT(ERROR, "pthread_mutex_lock()");
                return NULL;
            }
            while((e = sm_dequeue2_low(q)) == NULL) {
                if (pthread_cond_wait(&(q->empty), &(q->queue1_lock)) != EXIT_SUCCESS) {
                    REPORT(ERROR, "pthread_cond_wait()");
                    return NULL;
                }
            }
            if(pthread_mutex_unlock(&(q->queue1_lock)) != EXIT_SUCCESS) {
                REPORT(ERROR, "pthread_mutex_unlock()");
                sm_enqueue2(e, q);
                e = NULL;
            }
        }
    
    return e;
}



