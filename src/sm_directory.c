/* SM.EXEC
   Directory
   (c) anton.bondarenko@gmail.com */

#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>

#include "../oam/logger.h"
#include "sm_directory.h"

// Private methods

static sm_directory *find_record(sm_directory *t, const char *name) {
	sm_directory *r = t;
	while (r != NULL && strcmp(r->name, name)){
		r = r->next;
	}
	return r;
}

// Public methods

sm_directory *sm_directory_create() {
	return NULL;
}
		
sm_directory *sm_directory_set(sm_directory *t, const char *name, void *ptr){
	sm_directory *r = find_record(t, name);
	if(t == NULL || r == NULL) {
		if((r = malloc(sizeof(sm_directory))) == NULL) {
        	REPORT(ERROR, "malloc()");
        	return NULL;
		}
		if((r->name = malloc(strlen(name))) == NULL) {
        	REPORT(ERROR, "malloc()");
			free(r);
			return NULL;
    	}
		strcpy(r->name, name);
		r->ref = &(r->ptr);
		r->ptr = ptr;
		r->prev = NULL;
		r->next = t;
		if(t != NULL)
			r->next->prev = r;
		t = r;
    }
	else {
		char *r_n = r->name;
		if((r->name = malloc(strlen(name))) == NULL) {
        	REPORT(ERROR, "malloc()");
			r->name = r_n;
			return NULL;
    	}
		free(r_n);
		strcpy(r->name, name);
		r->ptr = ptr;
	}
	return t;
}
	
void **sm_directory_get_ref(sm_directory *t, const char *name) {	
	sm_directory *tr = find_record(t, name);
	if (tr == NULL)
		return NULL;
	else
		return tr->ref;
}

void sm_directory_remove(sm_directory *t, const char *name) {
	sm_directory *tr = find_record(t, name);
	if (tr != NULL) {
		tr->prev->next = tr->next;
		tr->next->prev = tr->prev;
		free(tr->name);
		free(tr);
	}
}

void sm_directory_free(sm_directory *t) {
	sm_directory *tmp;
	while(t != NULL) {
		tmp = t->next;
		free(t);
		t = tmp;
	}
}