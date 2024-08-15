/* SM.EXEC
   Apply event function
   (c) anton.bondarenko@gmail.com */

#include <stdio.h>
#include <unistd.h>
#include "sm_apply.h"
#include "sm_tx.h"
#include "sm_array.h"
#include "sm_logger.h"
#include "sm_fsm.h"

extern __thread void *__sm_tx_desc;

// Private methods
static void park_node(sm_state *s)
{
    sm_fsm_node *n = sm_fsm_get_node(s);
    if (n->type == SM_FINAL)
    {
        s->state_id = (*(s->fsm))->initial;
    }
    if (SM_STATE_DEPOT(s) != NULL)
    {
        if (n->type == SM_FINAL)
        {
            sm_array_release_state(&s);
        }
        else
        {
            sm_array_park_state(&s);
        }
    }
}

// Public methods
void sm_apply_event(sm_event *e, sm_state *s)
{
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
    do
    {

        // Reconcile thread-worker object address
        SM_STATE_TX(s) = __sm_tx_desc;

        done = false;
        if ((n = sm_fsm_get_node(s)) == NULL)
        {
            sprintf(error, "Couldn't find SM node, State id = %d, SM name = %s", s->state_id, (*(s->fsm))->name);
            SM_SYSLOG_CAUSE(SM_CORE, SM_LOG_ERR, "sm_state_get_node(s) == NULL", error);
            return;
        }
        if ((t = sm_fsm_get_transition(e, s)) == NULL)
        {
            sprintf(error, "Couldn't find SM transition, State Id = %d, Event Id = %d, SM name = %s",
                    s->state_id, e->event_id, (*(s->fsm))->name);
            SM_SYSLOG_CAUSE(SM_CORE, SM_LOG_ERR, "sm_state_get_transition(e, s) == NULL", error);
            return;
        }
        switch (n->type)
        {
        case SM_STATE:
        case SM_INITIAL:
            switch (t->type)
            {
            case SM_REGULAR:
                result = (*(sm_app *)t->transitionRef)(e, s);
                if (result != EXIT_SUCCESS)
                {
                    app_name = sm_directory_get_name(SM_STATE_TX(s)->exec->dir, *(sm_app *)t->transitionRef);
                    sprintf(error, "Application %s returned error code %d", app_name, result);
                    SM_SYSLOG_CAUSE(SM_CORE, SM_LOG_ERR, "Application error", error);
                }
                s->state_id = t->targetNode;
                if (t->setEventId)
                {
                    e->event_id = t->setEventId;
                }
                next_n = sm_fsm_get_node(s);
                if (next_n->type != SM_PROCESS)
                {
                    if (!sm_tx_stack_empty(SM_STATE_TX(s)))
                    {
                        sm_tx_pop_state(SM_STATE_TX(s));
                        s = SM_STATE_TX(s)->state;
                        park_node(s);
                    }
                    else
                    {
                        park_node(s);
                        done = true;
                    }
                }
                break;

            case SM_HIERARCHICAL:
                s_new = sm_array_get_state(*(sm_array **)t->transitionRef, SM_EVENT_HASH_KEY(e)->string, SM_EVENT_HASH_KEY(e)->length);
                if (s_new != NULL)
                {
                    s->state_id = t->targetNode;
                    s = s_new;
                    if (t->setEventId)
                    {
                        e->event_id = t->setEventId;
                    }
                    sm_tx_push_state(SM_STATE_TX(s), &s);
                }
                else
                {
                    SM_SYSLOG_CAUSE(SM_CORE, SM_LOG_ERR, "Event processing error",
                              "Hierarchical call failed - system cannot get state in the array");
                }
                /*s->state_id = t->targetNode;
                  if(t->setEventId) {
                      e->event_id = t->setEventId;
                  }
                  s = sm_array_get_state(*(sm_array **)t->transitionRef, e->key, e->key_length);
                  sm_tx_push_state(SM_STATE_TX(s), &s);*/
                break;

            case SM_CASCADE:
                if (!sm_tx_stack_empty(SM_STATE_TX(s)))
                {
                    SM_SYSLOG_CAUSE(SM_CORE, SM_LOG_ERR, "Non-empty stack on SM cascading",
                              "SM processing error");
                }
                s->state_id = t->targetNode;
                if (t->setEventId)
                {
                    e->event_id = t->setEventId;
                }
                s = sm_array_get_state(*(sm_array **)t->transitionRef, SM_EVENT_HASH_KEY(e)->string, SM_EVENT_HASH_KEY(e)->length);
                park_node(s);
                break;

            case SM_ASYNC:
                if (!sm_tx_stack_empty(SM_STATE_TX(s)))
                {
                    SM_SYSLOG_CAUSE(SM_CORE, SM_LOG_ERR, "Non-empty stack on SM async chaining",
                              "SM processing error");
                }
                s->state_id = t->targetNode;
                if (t->setEventId)
                {
                    e->event_id = t->setEventId;
                }
                e->ctl.D = false;
                park_node(s);
                sm_lock_enqueue2(*(sm_queue2 **)t->transitionRef, &e);
                done = true;
                break;

            case SM_STATIC:
                s->fsm = (sm_fsm **)t->transitionRef;
                s->state_id = t->targetNode;
                if (t->setEventId)
                {
                    e->event_id = t->setEventId;
                }
                break;

            default:
                SM_SYSLOG_CAUSE(SM_CORE, SM_LOG_ERR, "Event processing error",
                          "Unknown transition reference type");
                break;
            }
            break;

        case SM_PROCESS:
            if (t->type == SM_REGULAR)
            {
                result = (*(sm_app *)t->transitionRef)(e, s);
                if (result != EXIT_SUCCESS)
                {
                    app_name = sm_directory_get_name(SM_STATE_TX(s)->exec->dir, (*(sm_app *)t->transitionRef));
                    sprintf(error, "Application %s returned error code %d", app_name, result);
                    SM_SYSLOG_CAUSE(SM_CORE, SM_LOG_ERR, "Application error", error);
                }
                s->state_id = t->targetNode;
                if (t->setEventId)
                {
                    e->event_id = t->setEventId;
                }
                next_n = sm_fsm_get_node(s);
                if (next_n->type != SM_PROCESS)
                {
                    if (!sm_tx_stack_empty(SM_STATE_TX(s)))
                    {
                        sm_tx_pop_state(SM_STATE_TX(s));
                        s = SM_STATE_TX(s)->state;
                        park_node(s);
                    }
                    else
                    {
                        park_node(s);
                        done = true;
                    }
                }
            }
            else if (t->type == SM_HIERARCHICAL)
            {
                sm_state *s_new = sm_array_get_state(*(sm_array **)t->transitionRef, SM_EVENT_HASH_KEY(e)->string, SM_EVENT_HASH_KEY(e)->length);
                if (s_new != NULL)
                {
                    s->state_id = t->targetNode;
                    s = s_new;
                    if (t->setEventId)
                    {
                        e->event_id = t->setEventId;
                    }
                    sm_tx_push_state(SM_STATE_TX(s), &s);
                }
                else
                {
                    SM_SYSLOG_CAUSE(SM_CORE, SM_LOG_ERR, "Event processing error",
                              "Hierarchical call failed - system cannot get state in the array");
                }
            }
            else
            {
                SM_SYSLOG_CAUSE(SM_CORE, SM_LOG_ERR, "Event processing error",
                          "Unsupported transition from PROCESS node");
            }
            break;

        case SM_FINAL:
            SM_SYSLOG_CAUSE(SM_CORE, SM_LOG_ERR, "Event processing error",
                      "Transition from FINAL state");
            break;

        default:
            SM_SYSLOG_CAUSE(SM_CORE, SM_LOG_ERR, "Event processing error",
                      "Unknown transition type");
            break;
        }
    } while (!done);
}
