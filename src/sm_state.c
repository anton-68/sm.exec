/* SM.EXEC
   State
   anton.bondarenko@gmail.com */

#include <string.h>         //memcmp
#include "sm_state.h"
#include "sm_queue.h"
//#include "sm_memory.h"

//public methods

bool sm_state_key_match(sm_state *s, const void *key, size_t key_length){
    if(key_length != s->key_length)
        return false;
    else
        return (memcmp(s->key, key, key_length) == 0);
}

int sm_state_set_key(sm_state *s, const void *key, size_t key_length) {
    memset(s->key, '\0', SM_STATE_HASH_KEYLEN);
    s->key_length = MIN(key_length, SM_STATE_HASH_KEYLEN);
    memcpy(s->key, key, MIN(key_length, s->key_length));
    return EXIT_SUCCESS;
}
    
sm_state *sm_state_create(sm_fsm **f, size_t payload_size) {
    if(!f){
        REPORT(ERROR, "Empty SM on input");
        return NULL;
    }
    sm_state *s;
    if((s = malloc(sizeof(sm_state))) == NULL) {
        REPORT(ERROR, "malloc()");
        return s;
    }
    s->data_size = payload_size;
    if(s->data_size > 0) {
        if ((s->data = malloc(s->data_size)) == NULL) {
            REPORT(ERROR, "malloc()");
            free(s);
            return NULL;
        }
        else
            memset(s->data, '\0', s->data_size);
    }
    else {
        s->data = NULL;
    }
    s->fsm = f; 
    if(*f != NULL)
        s->id = (*f)->initial;
    else
        s->id = 0;
    s->next = NULL;
    if(SM_STATE_HASH_KEYLEN > 0) {
        if ((s->key = malloc(SM_STATE_HASH_KEYLEN)) == NULL) {
            REPORT(ERROR, "malloc()");
            free(s);
            return NULL;
        }
        else 
            memset(s->key, '\0', SM_STATE_HASH_KEYLEN);
    }
    else {
        s->key = NULL;
    }
    s->key_length = 0;
    s->key_hash = 0;
    s->home = NULL;
    s->tx = NULL;
    s->trace = NULL;
    return s;    
}

void sm_state_purge(sm_state *s) {
    if(s == NULL)
        return;
    memset(s->key, '\0', SM_STATE_HASH_KEYLEN);
    s->key_length = 0;
    s->key_hash = 0;
    s->id = 0;
    s->fsm = NULL;
    s->next = NULL; // ??
    memset(s->data, '\0', s->data_size);
    while(s->trace != NULL) {
        sm_event *ne = s->trace->next;
        s->trace->disposable = true;
        for (int stage = 0; stage < SM_NUM_OF_PRIORITY_STAGES; stage++)
            ne->priority[stage] = 0;
        sm_queue_enqueue(s->trace, s->trace->home);
        s->trace = ne;
    }
}

void sm_state_clear(sm_state *s) {
    if(s == NULL)
        return;
    if(s->key != NULL) {    
        memset(s->key, '\0', s->key_length);
    }
    s->key_length = 0;
    s->key_hash = 0;
    s->id = (*(s->fsm))->initial;
    //s->fsm = NULL;
    s->next = NULL; // ??
    memset(s->data, '\0', s->data_size);
    while(s->trace != NULL) {
        sm_event *ne = s->trace->next;
        s->trace->disposable = true;
        for (int stage = 0; stage < SM_NUM_OF_PRIORITY_STAGES; stage++)
            ne->priority[stage] = 0;
        sm_queue_enqueue(s->trace, s->trace->home);
        s->trace = ne;
    }
}

void sm_state_free(sm_state * s) {
    if(s == NULL)
        return;
    free(s->data);
    while(s->trace != NULL) {
        sm_event *ne = s->trace->next;
        s->trace->disposable = true;
        sm_queue_enqueue(s->trace, s->trace->home);
        s->trace = ne;
    }
    free(s);
}

sm_fsm_node *sm_state_get_node(sm_state *s) {
    sm_fsm *f = *(s->fsm);
    size_t i;
    for(i = 0; i < f->num_of_nodes && f->nodes[i].id != s->id; i++);
    if (i == f->num_of_nodes) {
        return NULL;
    }
    else {
        return &(f->nodes[i]);
    }
}

sm_fsm_transition *sm_state_get_transition(sm_event *e, sm_state *s) {
    sm_fsm_node *n = sm_state_get_node(s);
    size_t i;
    for(i = 0; i < n->num_of_transitions && n->transitions[i].appliedOnEvent != e->id; i++);
    if (i == n->num_of_transitions) {
        for(i = 0; i < n->num_of_transitions && n->transitions[i].appliedOnEvent != 0; i++);
    }
    if (i < n->num_of_transitions) {
        return &(n->transitions[i]);
    }
    else {
        SM_SYSLOG(SM_CORE, SM_LOG_ERR, "Cannot find transition in SM", "No default transition found");
        return NULL;
    }
}


// DEPRECATED: backward compatibility
/*
void sm_apply_event(sm_state *s, sm_event *e){
    if(sm_fsm_apply_event(s, e) != EXIT_SUCCESS)
        SM_SYSLOG(SM_CORE, SM_LOG_ERR, "Error invoking sm_fsm_apply_event", "(?) Misconfigured FSM");
}
*/
