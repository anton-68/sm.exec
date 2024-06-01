/* SM.EXEC
   Apps
   (c) anton.bondarenko@gmail.com */

// #include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>

#include "sm_logger.h"
#include "sm_app.h"

// DEPRECATED [
// Private methods

static sm_app_table *find_record(sm_app_table *t, const char *name) {
	sm_app_table *r = t;
	while (r != NULL && strcmp(r->name, name)){
		r = r->next;
	}
	return r;
}

// Public methods

sm_app_table *sm_app_table_create() {
/*	sm_app_table *t;
    if((t = malloc(sizeof(sm_app_table))) == NULL) {
        REPORT(ERROR, "malloc()");
        return NULL;
    } 
	t->name = NULL;
	t->app = NULL;
	t->prev = NULL;
	t->next = NULL;
	return t;          */
	return NULL;
}
		
sm_app_table *sm_app_table_set(sm_app_table *t, const char *name, sm_app app){
	sm_app_table *r;
	if(t == NULL)
		r = NULL;
	else 
		r = find_record(t, name);
	if (r == NULL) {
		if((r = malloc(sizeof(sm_app_table))) == NULL) {
        	REPORT(ERROR, "malloc()");
        	return NULL;
		}
		if((r->name = malloc(strlen(name))) == NULL) {
        	REPORT(ERROR, "malloc()");
			free(r);
			return NULL;
    	}
		strcpy(r->name, name);
		r->ref = &(r->app);
		r->app = app;
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
		r->app = app;
	}
	return t;
}
	
sm_app *sm_app_table_get_ref(sm_app_table *t, const char *name) {	
	sm_app_table *tr = find_record(t, name);
	if (tr == NULL)
		return NULL;
	else
		return tr->ref;
}

void sm_app_table_remove(sm_app_table *t, const char *name) {
	sm_app_table *tr = find_record(t, name);
	if (tr != NULL) {
		tr->prev->next = tr->next;
		tr->next->prev = tr->prev;
		free(tr->name);
		free(tr);
	}
}

void sm_app_table_free(sm_app_table *t) {
	sm_app_table *tmp;
	while(t != NULL) {
		tmp = t->next;
		free(t);
		t = tmp;
	}
}
//]
