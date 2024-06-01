/* SM.EXEC
   Apply event function
   (c) anton.bondarenko@gmail.com */

#include <stdio.h>
#include <unistd.h>
#include "sm_apply.h"
#include "sm_tx.h"
#include "sm_array.h"
#include "sm_app.h"
#include "sm_logger.h"
#include "sm_fsm.h"

extern __thread void * __sm_tx_desc;
    
// Private methods
static void park_node(sm_state *s) {
    sm_fsm_node *n = sm_state_get_node(s);
    if(n->type == SM_FINAL) {
        s->id = (*(s->fsm))->initial;
    }
    if(s->home != NULL) {
        if(n->type == SM_FINAL) {
            sm_array_release_state(s->home, s);
        }
        else { 
            sm_array_depot_state(s->home, s);
        }
    }
}

// Public methods
void sm_apply_event(sm_event *e, sm_state *s) {
#ifdef SM_APPLY_DELAY_MS
    usleep(SM_APPLY_DELAY_MS);
#endif
    char *app_name;
    char error[80];
    sm_fsm_node *n; 
    sm_fsm_node *next_n;
    sm_fsm_transition *t;
    sm_state *s_new;
    bool done;
    int result;
    do {

        // Reconcile thread-worker object address
        s->tx = __sm_tx_desc;
        
        done = false;
        if((n = sm_state_get_node(s)) == NULL) {
            sprintf(error, "Couldn't find SM node, State id = %ld, SM name = %s", s->id, (*(s->fsm))->name); 
            SM_SYSLOG(SM_CORE, SM_LOG_ERR, error,"sm_state_get_node(s) == NULL"); 
            return;
        }
        if((t = sm_state_get_transition(e, s)) == NULL) {
            sprintf(error, "Couldn't find SM transition, State Id = %ld, Event Id = %ld, SM name = %s", 
                    s->id, e->id, (*(s->fsm))->name);
            SM_SYSLOG(SM_CORE, SM_LOG_ERR, error, "sm_state_get_transition(e, s) == NULL"); 
            return;
        }
        switch (n->type) {
            case SM_STATE:
            case SM_INITIAL:
                switch (t->type) {
                    case SM_REGULAR:
                        result = (*(sm_app *)t->transitionRef)(e, s);
                        if(result != EXIT_SUCCESS) {
                            app_name = sm_directory_get_name(s->tx->exec->dir, *(sm_app *)t->transitionRef);
                            sprintf(error, "Application %s returned error code %d", app_name, result); 
                            SM_SYSLOG(SM_CORE, SM_LOG_ERR, error, "Application error");
                        }
                        s->id = t->targetNode;
                        if(t->setEventId) {
                            e->id = t->setEventId;
                        }
                        next_n = sm_state_get_node(s);
                        if(next_n->type != SM_PROCESS) {
                            if(!sm_tx_stack_empty(s->tx)) {
                                sm_tx_pop_state(s->tx);
                                s = s->tx->state;
                                park_node(s);
                            }
                            else {
                                park_node(s);
                                done = true;
                            }
                        }
                        break;

                    case SM_HIERARCHICAL:
                        s_new = sm_array_get_state(*(sm_array **)t->transitionRef, e->key, e->key_length);
                        if(s_new != NULL) {
                            s->id = t->targetNode;
                            s = s_new;
                            if(t->setEventId) {
                                e->id = t->setEventId;
                            }
                            sm_tx_push_state(s->tx, s);
                        }
                        else {
                            SM_SYSLOG(SM_CORE, SM_LOG_ERR, "Event processing error", 
                            "Hierarchical call failed - system cannot get state in the array");
                        }   
                      /*s->id = t->targetNode;
                        if(t->setEventId) {
                            e->id = t->setEventId;
                        }
                        s = sm_array_get_state(*(sm_array **)t->transitionRef, e->key, e->key_length);
                        sm_tx_push_state(s->tx, s);*/
                        break;
                    
                    case SM_CASCADE:
                        if(!sm_tx_stack_empty(s->tx)) {
                            SM_SYSLOG(SM_CORE, SM_LOG_ERR, "Non-empty stack on SM cascading", 
                                                             "SM processing error");
                        }
                        s->id = t->targetNode;
                        if(t->setEventId) {
                            e->id = t->setEventId;
                        }
                        s = sm_array_get_state(*(sm_array **)t->transitionRef, e->key, e->key_length);
                        park_node(s);
                        break;

                    case SM_ASYNC:
                        if(!sm_tx_stack_empty(s->tx)) {
                            SM_SYSLOG(SM_CORE, SM_LOG_ERR, "Non-empty stack on SM async chaining", 
                                                             "SM processing error");
                        }
                        s->id = t->targetNode;
                        if(t->setEventId) {
                            e->id = t->setEventId;
                        }
                        e->disposable = false;
                        park_node(s);
                        sm_lock_enqueue2(e, *(sm_queue2 **)t->transitionRef);
                        done = true;
                        break;

                    case SM_STATIC:
                        s->fsm = (sm_fsm **)t->transitionRef;
                        s->id = t->targetNode;
                        if(t->setEventId) {
                            e->id = t->setEventId;
                        }
                        break;

                    default :
                        SM_SYSLOG(SM_CORE, SM_LOG_ERR, "Event processing error", 
                                                         "Unknown transition reference type");
                        break;
                }
                break;

            case SM_PROCESS:
                if(t->type == SM_REGULAR) {
                    result = (*(sm_app *)t->transitionRef)(e, s);
                    if(result != EXIT_SUCCESS) {
                        app_name = sm_directory_get_name(s->tx->exec->dir, (*(sm_app *)t->transitionRef));
                        sprintf(error, "Application %s returned error code %d", app_name, result); 
                        SM_SYSLOG(SM_CORE, SM_LOG_ERR, error, "Application error");
                    }
                    s->id = t->targetNode;
                    if(t->setEventId) {
                        e->id = t->setEventId;
                    }
                    next_n = sm_state_get_node(s);
                    if(next_n->type != SM_PROCESS) {
                            if(!sm_tx_stack_empty(s->tx)) {
                                sm_tx_pop_state(s->tx);
                                s = s->tx->state;
                                park_node(s);
                            }
                            else {
                                park_node(s);
                                done = true;
                            }
                        }
                }
                else if (t->type == SM_HIERARCHICAL) {
                    sm_state *s_new = sm_array_get_state(*(sm_array **)t->transitionRef, e->key, e->key_length);
                    if(s_new != NULL) {
                        s->id = t->targetNode;
                        s = s_new;
                        if(t->setEventId) {
                            e->id = t->setEventId;
                        }
                        sm_tx_push_state(s->tx, s);
                    }
                    else {
                        SM_SYSLOG(SM_CORE, SM_LOG_ERR, "Event processing error", 
                        "Hierarchical call failed - system cannot get state in the array");
                    }   
                }
                else {
                    SM_SYSLOG(SM_CORE, SM_LOG_ERR, "Event processing error", 
                                                     "Unsupported transition from PROCESS node");
                }
                break;

            case SM_FINAL:
                SM_SYSLOG(SM_CORE, SM_LOG_ERR, "Event processing error", 
                                                 "Transition from FINAL state");
                break;

            default :
                SM_SYSLOG(SM_CORE, SM_LOG_ERR, "Event processing error", 
                                                 "Unknown transition type");
                break;
        }
    } while (!done);
}
