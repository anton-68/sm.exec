/* SM.EXEC
   State array   
   anton.bondarenko@gmail.com */

#include "sm_array.h"






/* State Array */

// Private methods

sm_state *sm_array_find_by_hash(sm_array *d, HASH_TYPE h, void *const key, size_t key_length);
sm_state *sm_array_get_by_hash(sm_array *d, HASH_TYPE h, void *const key, size_t key_length);
sm_state *sm_array_pop(sm_array *d);
int sm_array_push(sm_array *d, sm_state *c);
void sm_array_table_free(sm_array *d);
void sm_array_stack_free(sm_array *d); 
int sm_array_insert_into_hash(sm_array *d, sm_state *c); 
int sm_array_remove_from_hash(sm_array *d, sm_state *c); 






// Public methods

sm_array *sm_array_create(size_t stack_size, size_t state_size, bool synchronized){
	// Allocate depot main structure
    sm_array *d;
    if((d = malloc(sizeof(sm_array))) == NULL) {
        REPORT(ERROR, "malloc()");
        return NULL;
    }
	d->stack_size = stack_size;
	// Calculate size as the nearest power of 2
    d->hash_size = hashsize(d->stack_size);
	// Calculate mask 
	d->hash_mask = hashmask(d->stack_size);
	// Allocate stack of state addresses
	if((d->stack = malloc(d->hash_size * sizeof(sm_state *))) == NULL) {
        REPORT(ERROR, "malloc()");
        free(d);
        return NULL;
    }
	// Allocate state bodies
	int i;
	sm_state *c;
	for(i = 0; i < d->stack_size; i++) {
        if((c = sm_state_create(state_size)) == NULL) {
            REPORT(ERROR, "state_create()");
            sm_array_stack_free(d);
            return NULL;
        }
        sm_array_push(d, c);
    }
	// Initialize pointer for stack of state IDs   
	d->next_free = 0;
	// Allocate hash array state pointers   
	if((d->table = malloc(d->hash_size * sizeof(sm_state *))) == NULL) {
        REPORT(ERROR, "malloc()");
        free(d);
        return NULL;
    }
	// Initialize hash function	
	d->hash_function = &hashlittle;
	// Initialize 'previous' hash value	
	d->last_hash_value = 0;
	// Initialize sybchronization flag	
	d->synchronized = synchronized;
	if(d->synchronized) {
		pthread_mutexattr_t attr;	
    	if(pthread_mutexattr_init(&attr) != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_mutexattr_init()");
        	sm_array_free(d);
        	return NULL;
    	}
    	if(pthread_mutexattr_settype(&attr, TL_MUTEX_TYPE) != EXIT_SUCCESS){
        	REPORT(ERROR, "pthread_mutexattr_settype()");
        	sm_array_free(d);
        	return NULL;
    	}    
    	if(pthread_mutex_init(&(d->table_lock),&attr) != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_mutex_init()");
        	sm_array_free(d);
        	return NULL;
		}
		if(pthread_mutex_init(&(d->stack_lock),&attr) != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_mutex_init()");
        	sm_array_free(d);
        	return NULL;
		}
		if(pthread_cond_init(&(d->empty), NULL) != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_cond_init()");
        	pthread_cond_signal(&d->empty);
        	sm_array_free(d);
        	return NULL;
    	}
	}
    return d;
}

void sm_array_free(sm_array *d){
	if(d->synchronized)
    	pthread_mutex_destroy(&d->table_lock);
	if(d->synchronized)
    	pthread_mutex_destroy(&d->stack_lock); 
	sm_array_table_free(d);	
    free(d->table);
	sm_array_stack_free(d);
	free(d->stack);	
    free(d);
}

size_t sm_array_stack_size(sm_array *q) {}

sm_state *sm_array_find_state(sm_array *d, void *const key, size_t key_length){
    HASH_TYPE h = d->hash_function(key, key_length, d->last_hash_value) & d->hash_mask;
	d->last_hash_value = h;
	return sm_array_find_by_hash(d, h, key, key_length);
}

sm_state *sm_array_get_state(sm_array *d, void *const key, size_t key_length){
	HASH_TYPE h = d->hash_function(key, key_length, d->last_hash_value) & d->hash_mask;
	d->last_hash_value = h;
	return sm_array_get_by_hash(d, h, key, key_length);	
}
		
void sm_array_release_state(sm_array *d, sm_state *c){
	sm_array_remove_from_hash(d, c);
	sm_state_purge(c);
	sm_array_push(d, c);
}	
		
// Private methods
		
sm_state *sm_array_find_by_hash(sm_array *d, HASH_TYPE h, void *const key, size_t key_length){
	if (d->synchronized)
		if(pthread_mutex_lock(&(d->table_lock)) != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_mutex_lock()");
        	return NULL;
    	} 
	sm_state *c = d->table[h];	
	while(c != NULL && !sm_state_key_match(c, key, key_length))
		  c = c->next;
	if (d->synchronized)
		if(pthread_mutex_unlock(&(d->table_lock)) != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_mutex_unlock()");
        	return NULL;
    	}
	return c;
}
		
sm_state *sm_array_get_by_hash(sm_array *d, HASH_TYPE h, void *const key, size_t key_length){	 
	sm_state *c = sm_array_find_by_hash(d, h, key, key_length);
	if (c == NULL) {
		sm_state *c = sm_array_pop(d);
		c->key_hash = h;
		c->key = key;
		c->key_length = key_length;
	}
	if(sm_array_insert_into_hash(d, c) != EXIT_SUCCESS){
		sm_array_push(d, c); 
		return NULL;
	}
	return c;
}		
		
sm_state *sm_array_pop(sm_array *d){
	if (d->synchronized) {
		if(pthread_mutex_lock(&(d->stack_lock)) != EXIT_SUCCESS){
        	REPORT(ERROR, "pthread_mutex_lock()");
        	return NULL;
    	}
		while(d->next_free >= d->hash_size) 
        	if (pthread_cond_wait(&(d->empty), &(d->stack_lock)) != EXIT_SUCCESS){
            	REPORT(ERROR, "pthread_cond_wait()");
            	return NULL;
        	}
	}
	sm_state *c = d->stack[d->next_free];	
	d->next_free++;
	if (d->synchronized)
		if(pthread_mutex_unlock(&(d->stack_lock)) != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_mutex_unlock()");
        	return NULL;
    	}		
	return c; 
}	

int sm_array_push(sm_array *d, sm_state *c){
	if(d->next_free == 0)	
		return EXIT_FAILURE;
	if(d->synchronized)
		if(pthread_mutex_lock(&(d->stack_lock)) != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_mutex_lock()");
        	return EXIT_FAILURE;
    	}
	d->next_free--;
	d->stack[d->next_free] = c;
	if (d->synchronized)	{
  		if(pthread_cond_signal(&(d->empty)) != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_cond_signal()");
        	return EXIT_FAILURE;
    	}
    	if(pthread_mutex_unlock(&(d->stack_lock)) != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_mutex_unlock()");
        	return EXIT_FAILURE;
    	}
	} 
    return EXIT_SUCCESS;	
}

void sm_array_table_free(sm_array *d){
	int i;
	sm_state *c;
	for(i = 0; i < d->hash_size; i++){
		c = d->table[i];
		while(c != NULL) {
			sm_state_free(c);
			c = c->next;
		}
	}
}			


	
void sm_array_stack_free(sm_array *d){
	while(d->next_free > 0) {
		sm_state_free(d->stack[d->next_free]);
		d->next_free--;
	}	
}	
		
int sm_array_insert_into_hash(sm_array *d, sm_state *c){
	if(c == NULL) 
    	return EXIT_FAILURE;
	if (d->synchronized)
		if(pthread_mutex_lock(&(d->table_lock)) != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_mutex_lock()");
        	return EXIT_FAILURE;
    	} 
	c->next = d->table[c->key_hash];
	d->table[c->key_hash] = c;
	if (d->synchronized)
		if(pthread_mutex_unlock(&(d->table_lock)) != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_mutex_unlock()");
        	return EXIT_FAILURE;
    	}
	return EXIT_SUCCESS;
}		

int sm_array_remove_from_hash(sm_array *d, sm_state *c){	
	if (d->synchronized)
		if(pthread_mutex_lock(&(d->table_lock)) != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_mutex_lock()");
        	return EXIT_FAILURE;
    	} 
	if(c == d->table[c->key_hash])
		d->table[c->key_hash] == d->table[c->key_hash]->next;
	else{ 	
		sm_state *c1 = d->table[c->key_hash];	
		while(c1 != NULL && c1->next != c)
			c1 = c1->next;
		if(c1 != NULL)
			d->table[c->key_hash] == d->table[c->key_hash]->next;
	}
	if (d->synchronized)
		if(pthread_mutex_unlock(&(d->table_lock)) != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_mutex_unlock()");
        	return EXIT_FAILURE;
    	} 
	return EXIT_SUCCESS;
}			
		


