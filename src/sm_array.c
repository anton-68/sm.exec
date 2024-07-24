/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Array class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "sm_array.h"

// extern __thread void * __sm_tx_desc;

static sm_state *find_by_hash(sm_array *d, HASH_TYPE h, const void* key, size_t key_length);
static sm_state *get_by_hash(sm_array *d, HASH_TYPE h, const void * key, size_t key_length);
static sm_state *pop(sm_array *d);
static int push(sm_array *d, sm_state *c);
static void table_free(sm_array *d);
static void stack_free(sm_array *d); 
static int insert_into_hash(sm_array *d, sm_state *c); 
static int remove_from_hash(sm_array *d, sm_state *c);

sm_array *sm_array_create(size_t key_length, size_t state_size, sm_fsm **fsm, bool S, bool C, bool E, bool H)
{

    sm_array *a;
    if (SM_UNLIKELY(a = aligned_alloc(SM_WORD, sizeof(sm_array))) == NULL)
    {
        SM_REPORT_MESSAGE(SM_LOG_ERR, "malloc() failed");
        return NULL;
    }
    a->array_size = hashsize(key_length);
    a->mask = hashmask(key_length);
    a->hash_function = &hashlittle;
    
    sm_state *s;
    if (SM_UNLIKELY((s = sm_state_create(NULL, state_size, true, true, C, E, H)) == NULL))
    {
        SM_REPORT_MESSAGE(SM_LOG_ERR, "sm_event_create() returned NULL");
        free(a);
        return NULL;
    }
    a->queue_head = a->queue_tail = s;
    if (SM_UNLIKELY((a->queue_head->next = aligned_alloc(SM_WORD, a->array_size * sm_state_sizeof(s))) == NULL))
    {
        SM_REPORT_MESSAGE(SM_LOG_ERR, "aligned_alloc() failed");
        free(s);
        free(a);
        return NULL;
    }
    for (size_t i = 0; i < a->array_size; i++)
    {
        
    }

        // Initialize pointer for stack of state IDs
        d->next_free = d->table_size - 1;
    // Allocate state bodies
    int i;
    sm_state *c;
    d->synchronized = synchronized;
    // Initialize synchronization flag  
    if(d->synchronized) {
        pthread_mutexattr_t attr;   
        if(pthread_mutexattr_init(&attr) != EXIT_SUCCESS) {
            REPORT(SM_LOG_ERR, "pthread_mutexattr_init()");
            sm_array_free(d);
            return NULL;
        }
        if(pthread_mutexattr_settype(&attr, SM_MUTEX_TYPE) != EXIT_SUCCESS){
            REPORT(SM_LOG_ERR, "pthread_mutexattr_settype()");
            sm_array_free(d);
            return NULL;
        }    
        if(pthread_mutex_init(&(d->lock),&attr) != EXIT_SUCCESS) {
            REPORT(SM_LOG_ERR, "pthread_mutex_init()");
            sm_array_free(d);
            return NULL;
        }
        if(pthread_cond_init(&(d->empty), NULL) != EXIT_SUCCESS) {
            REPORT(SM_LOG_ERR, "pthread_cond_init()");
            pthread_cond_signal(&d->empty);
            sm_array_free(d);
            return NULL;
        }
    }
    for(i = 0; i < d->table_size; i++) {
        if((c = sm_state_create(fsm, state_size)) == NULL) {
            REPORT(SM_LOG_ERR, "state_create()");
            stack_free(d);
            return NULL;
        }
        SM_STATE_DEPOT(c) = d;
        push(d, c);
    }
    // Allocate hash array state pointers   
    if((d->table = calloc(d->table_size, sizeof(sm_state *))) == NULL) {
        REPORT(SM_LOG_ERR, "calloc()");
        free(d);
        return NULL;
    }
    return d;
}

void sm_array_free(sm_array *d){
    if(d->synchronized) {
//        pthread_mutex_destroy(&d->table_lock);
//        pthread_mutex_destroy(&d->stack_lock);
        pthread_mutex_destroy(&d->lock);
        pthread_cond_destroy(&d->empty);
    }
    table_free(d);  
    free(d->table);
    stack_free(d);
    free(d->stack); 
    free(d);
}

sm_state *sm_array_find_state(sm_array *d, const void *key, size_t key_length){
    HASH_TYPE h = d->hash_function(key, key_length, d->last_hash_value) & d->hash_mask;
//
    if (d->synchronized) {
        if(pthread_mutex_lock(&(d->lock)) != EXIT_SUCCESS) {
            REPORT(SM_LOG_ERR, "pthread_mutex_lock()");
            return NULL;
        } 
    }
//
    //d->last_hash_value = h;
    sm_state *s = find_by_hash(d, h, key, key_length);
//
    if (d->synchronized) {
        if(pthread_mutex_unlock(&(d->lock)) != EXIT_SUCCESS) {
            REPORT(SM_LOG_ERR, "pthread_mutex_unlock()");
            return NULL;
        }
    }
//
    return s;
}

sm_state *sm_array_get_state(sm_array *d, const void *key, size_t key_length){
    HASH_TYPE h = d->hash_function(key, key_length, d->last_hash_value) & d->hash_mask;
//
    if (d->synchronized) {
        if(pthread_mutex_lock(&(d->lock)) != EXIT_SUCCESS) {
            REPORT(SM_LOG_ERR, "pthread_mutex_lock()");
            return NULL;
        } 
    }
//
    //d->last_hash_value = h;
    sm_state *s = get_by_hash(d, h, key, key_length);
    s->tx = __sm_tx_desc;
//
    if (d->synchronized) {
        if(pthread_mutex_unlock(&(d->lock)) != EXIT_SUCCESS) {
            REPORT(SM_LOG_ERR, "pthread_mutex_unlock()");
            return NULL;
        }
    }
//
    return s;  
}
        
void sm_array_release_state(sm_array *d, sm_state *c){
//
    if (d->synchronized) {
        if(pthread_mutex_lock(&(d->lock)) != EXIT_SUCCESS) {
            REPORT(SM_LOG_ERR, "pthread_mutex_lock()");
            return;
        } 
    }
//
    remove_from_hash(d, c);
    sm_state_clear(c);
    c->tx = NULL;
    push(d, c);
//
    if (d->synchronized) {
        if(pthread_mutex_unlock(&(d->lock)) != EXIT_SUCCESS) {
            REPORT(SM_LOG_ERR, "pthread_mutex_unlock()");
            return;
        }
    }
//
}   
        
void sm_array_park_state(sm_array *d, sm_state *c){
//
    if (d->synchronized) {
        if(pthread_mutex_lock(&(d->lock)) != EXIT_SUCCESS) {
            REPORT(SM_LOG_ERR, "pthread_mutex_lock()");
            return;
        } 
    }
//
    c->tx = NULL;
//
    if (d->synchronized) {
        if(pthread_mutex_unlock(&(d->lock)) != EXIT_SUCCESS) {
            REPORT(SM_LOG_ERR, "pthread_mutex_unlock()");
            return;
        }
    }
//
}   
        
// Private methods
        
static sm_state *find_by_hash(sm_array *d, HASH_TYPE h, const void *key, size_t key_length){
/*    if (d->synchronized)
        if(pthread_mutex_lock(&(d->table_lock)) != EXIT_SUCCESS) {
            REPORT(SM_LOG_ERR, "pthread_mutex_lock()");
            return NULL;
        } */
    sm_state *c = d->table[h];  
    while(c != NULL && !sm_state_key_match(c, key, key_length))
          c = c->next;
/*    if (d->synchronized)
        if(pthread_mutex_unlock(&(d->table_lock)) != EXIT_SUCCESS) {
            REPORT(SM_LOG_ERR, "pthread_mutex_unlock()");
            return NULL;
        }*/
    return c;
}
        
static sm_state *get_by_hash(sm_array *d, HASH_TYPE h, const void *key, size_t key_length){  
    sm_state *s = find_by_hash(d, h, key, key_length);
    if (s == NULL) {
        if((s = pop(d)) == NULL)
            return NULL;
        s->key_hash = h;
        sm_state_set_key(s, key, key_length);
    }
    if(insert_into_hash(d, s) != EXIT_SUCCESS){
        push(d, s); 
        return NULL;
    }
    return s;
}       
        
static sm_state *pop(sm_array *d){
    if(d->next_free == d->table_size - 1)
        return NULL;
    if (d->synchronized) {
    /*    if(pthread_mutex_lock(&(d->stack_lock)) != EXIT_SUCCESS){
            REPORT(SM_LOG_ERR, "pthread_mutex_lock()");
            return NULL;
        }*/
        while(d->next_free >= d->table_size) 
            if (pthread_cond_wait(&(d->empty), &(d->lock/*stack_lock*/)) != EXIT_SUCCESS){
                REPORT(SM_LOG_ERR, "pthread_cond_wait()");
                return NULL;
            }
    }
    else
        if(d->next_free + 1 > d->table_size)
            return NULL;
    d->next_free++;
    sm_state *c = d->stack[d->next_free];   
/*    if (d->synchronized)
        if(pthread_mutex_unlock(&(d->stack_lock)) != EXIT_SUCCESS) {
            REPORT(SM_LOG_ERR, "pthread_mutex_unlock()");
            return NULL;
        }   */    
    return c; 
}   

static int push(sm_array *d, sm_state *c){
    if(d->next_free == -1)  
        return EXIT_FAILURE;
/*    if(d->synchronized) {
        if(pthread_mutex_lock(&(d->stack_lock)) != EXIT_SUCCESS) {
            REPORT(SM_LOG_ERR, "pthread_mutex_lock()");
            return EXIT_FAILURE;
        }
    }*/
    d->stack[d->next_free] = c;
    d->next_free--;
    if (d->synchronized) {
        if(pthread_cond_signal(&(d->empty)) != EXIT_SUCCESS) {
            REPORT(SM_LOG_ERR, "pthread_cond_signal()");
            return EXIT_FAILURE;
        }
/*        if(pthread_mutex_unlock(&(d->stack_lock)) != EXIT_SUCCESS) {
            REPORT(SM_LOG_ERR, "pthread_mutex_unlock()");
            return EXIT_FAILURE;
        }*/
    } 
    return EXIT_SUCCESS;    
}

static void table_free(sm_array *d){
    int i;
    sm_state *c;
    for(i = 0; i < d->table_size; i++){
        c = d->table[i];
        while(c != NULL) {
            sm_state_free(c);
            c = c->next;
        }
    }
}           

static void stack_free(sm_array *d){
    while(d->next_free > 0) {
        sm_state_free(d->stack[d->next_free]);
        d->next_free--;
    }   
}   
        
static int insert_into_hash(sm_array *d, sm_state *c){
    if(c == NULL) 
        return EXIT_FAILURE;
/*    if (d->synchronized)
        if(pthread_mutex_lock(&(d->table_lock)) != EXIT_SUCCESS) {
            REPORT(SM_LOG_ERR, "pthread_mutex_lock()");
            return EXIT_FAILURE;
        } */
    c->next = d->table[c->key_hash];
    d->table[c->key_hash] = c;
/*    if (d->synchronized)
        if(pthread_mutex_unlock(&(d->table_lock)) != EXIT_SUCCESS) {
            REPORT(SM_LOG_ERR, "pthread_mutex_unlock()");
            return EXIT_FAILURE;
        }*/
    return EXIT_SUCCESS;
}       

static int remove_from_hash(sm_array *d, sm_state *c){  
/*    if (d->synchronized)
        if(pthread_mutex_lock(&(d->table_lock)) != EXIT_SUCCESS) {
            REPORT(SM_LOG_ERR, "pthread_mutex_lock()");
            return EXIT_FAILURE;
        }*/ 
    if(c == d->table[c->key_hash])
        d->table[c->key_hash] = d->table[c->key_hash]->next;
    else{   
        sm_state *c1 = d->table[c->key_hash];   
        while(c1 != NULL && c1->next != c)
            c1 = c1->next;
        if(c1 != NULL)
            d->table[c->key_hash] = d->table[c->key_hash]->next;
    }
/*    if (d->synchronized)
        if(pthread_mutex_unlock(&(d->table_lock)) != EXIT_SUCCESS) {
            REPORT(SM_LOG_ERR, "pthread_mutex_unlock()");
            return EXIT_FAILURE;
        } */
    return EXIT_SUCCESS;
}           
        


