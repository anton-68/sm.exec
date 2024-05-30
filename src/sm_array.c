/* SM.EXEC
   State array   
   anton.bondarenko@gmail.com */

#include "sm_array.h"

/* State Array */

// Private methods

static sm_state *find_by_hash(sm_array *d, HASH_TYPE h, const void* key, size_t key_length);
static sm_state *get_by_hash(sm_array *d, HASH_TYPE h, const void * key, size_t key_length);
static sm_state *pop(sm_array *d);
static int push(sm_array *d, sm_state *c);
static void table_free(sm_array *d);
static void stack_free(sm_array *d); 
static int insert_into_hash(sm_array *d, sm_state *c); 
static int remove_from_hash(sm_array *d, sm_state *c); 

// Public methods

sm_array *sm_array_create(size_t stack_size, size_t state_size, sm_fsm **fsm, bool synchronized){
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
	// Initialize hash function	
	d->hash_function = &hashlittle;
	// Initialize 'previous' hash value	
	d->last_hash_value = 0;	
	if((d->stack = calloc(d->stack_size, sizeof(sm_state *))) == NULL) {
        REPORT(ERROR, "calloc()");
        free(d);
        return NULL;
    }
	// Initialize pointer for stack of state IDs   
	d->next_free = d->stack_size - 1;
	// Allocate state bodies
	int i;
	sm_state *c;
	d->synchronized = synchronized;
	// Initialize sybchronization flag	
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
	for(i = 0; i < d->stack_size; i++) {
        if((c = sm_state_create(fsm, state_size)) == NULL) {
            REPORT(ERROR, "state_create()");
            stack_free(d);
            return NULL;
        }
        push(d, c);
    }
	// Allocate hash array state pointers   
	if((d->table = calloc(d->hash_size, sizeof(sm_state *))) == NULL) {
        REPORT(ERROR, "calloc()");
        free(d);
        return NULL;
    }
    return d;
}

void sm_array_free(sm_array *d){
	if(d->synchronized)
    	pthread_mutex_destroy(&d->table_lock);
	if(d->synchronized)
    	pthread_mutex_destroy(&d->stack_lock); 
	table_free(d);	
    free(d->table);
	stack_free(d);
	free(d->stack);	
    free(d);
}

sm_state *sm_array_find_state(sm_array *d, const void *key, size_t key_length){
    HASH_TYPE h = d->hash_function(key, key_length, d->last_hash_value) & d->hash_mask;
	d->last_hash_value = h;
	return find_by_hash(d, h, key, key_length);
}

sm_state *sm_array_get_state(sm_array *d, const void *key, size_t key_length){
	HASH_TYPE h = d->hash_function(key, key_length, d->last_hash_value) & d->hash_mask;
	d->last_hash_value = h;
	return get_by_hash(d, h, key, key_length);	
}
		
void sm_array_release_state(sm_array *d, sm_state *c){
	remove_from_hash(d, c);
	sm_state_purge(c);
	push(d, c);
}	
		
// Private methods
		
static sm_state *find_by_hash(sm_array *d, HASH_TYPE h, const void *key, size_t key_length){
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
	if(d->next_free == d->stack_size - 1)
		return NULL;
	if (d->synchronized) {
		if(pthread_mutex_lock(&(d->stack_lock)) != EXIT_SUCCESS){
        	REPORT(ERROR, "pthread_mutex_lock()");
        	return NULL;
    	}
		while(d->next_free >= d->stack_size) 
        	if (pthread_cond_wait(&(d->empty), &(d->stack_lock)) != EXIT_SUCCESS){
            	REPORT(ERROR, "pthread_cond_wait()");
            	return NULL;
        	}
	}
	else
		if(d->next_free > d->stack_size - 1)
			return NULL;
	d->next_free++;
	sm_state *c = d->stack[d->next_free];	
	if (d->synchronized)
		if(pthread_mutex_unlock(&(d->stack_lock)) != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_mutex_unlock()");
        	return NULL;
    	}		
	return c; 
}	

static int push(sm_array *d, sm_state *c){
	if(d->next_free == -1)	
		return EXIT_FAILURE;
	if(d->synchronized) {
		if(pthread_mutex_lock(&(d->stack_lock)) != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_mutex_lock()");
        	return EXIT_FAILURE;
    	}
	}
	d->stack[d->next_free] = c;
	d->next_free--;
	if (d->synchronized) {
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

static void table_free(sm_array *d){
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

static void stack_free(sm_array *d){
	while(d->next_free > 0) {
		sm_state_free(d->stack[d->next_free]);
		d->next_free--;
	}	
}	
		
static int insert_into_hash(sm_array *d, sm_state *c){
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

static int remove_from_hash(sm_array *d, sm_state *c){	
	if (d->synchronized)
		if(pthread_mutex_lock(&(d->table_lock)) != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_mutex_lock()");
        	return EXIT_FAILURE;
    	} 
	if(c == d->table[c->key_hash])
		d->table[c->key_hash] = d->table[c->key_hash]->next;
	else{ 	
		sm_state *c1 = d->table[c->key_hash];	
		while(c1 != NULL && c1->next != c)
			c1 = c1->next;
		if(c1 != NULL)
			d->table[c->key_hash] = d->table[c->key_hash]->next;
	}
	if (d->synchronized)
		if(pthread_mutex_unlock(&(d->table_lock)) != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_mutex_unlock()");
        	return EXIT_FAILURE;
    	} 
	return EXIT_SUCCESS;
}			
		


