/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Apply module tests apps
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "apply_apps.h"
#include "../lib/parson/parson.h"

char *protocols[] = {
    "default",
    "ip",
    "dtmp",
    "sip",
    "rtsp",
    "serv"
};

char *commands[] = {
    "reserved",
    "originate",         // 1025
    "report",            // 1026
    "register",          // 1027
    "set_flow",          // 1028
    "request_flow",      // 1029
    "set_rule",          // 1030
    "get_rule",          // 1031
    "deploy_rule",       // 1032
    "receipt",           // 1033
    "pending"            // 1034
};

char *status_values[] = {
    "ok",                // 2048
    "blocked",           // 2049
    "allowed",           // 2050
    "unknown",           // 2051
    "skip"               // 2052
};

#define BLOCKED 2049
#define ALLOWED 2050
#define UNKNOWN 2051

char *sip_methods[] = {
    "reserved",
    "100 Trying",        // 3073
    "180 Ringing",       // 3074
    "200 OK",            // 3075
    "INVITE",            // 3076
    "CANCEL",            // 3077
    "BYE"                // 3078
};

char *end_nodes[] = {
    "Camera_1",
    "Camera_2",
    "TV_Set_1",
    "TV_Set_2",
    "GrannyWristyBand",
    "SmartVoice_1",
    "IPPhone_1",
    "IPPhone_2",
    "LightSensor_1",
    "TempSensor_1",
    "TempSensor_2",
    "Laptop_1",
    "Laptop_2",
    "SmartPhone_1",
    "SmartPhone_2"
};

char *roles[] = {
    "CCTVCam",
    "Smart_TV_Set",
    "IoT_Wearable",
    "SmartHome",
    "IPTelephony",
    "Client"
};

size_t assigned_role[] = {0, 0, 1, 1, 2, 3, 4, 4, 3, 3, 3, 5, 5, 5, 5};


extern char *sm_node_type_to_string[];
extern char *sm_transition_type_to_string[];

int recv(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)SM_EVENT_DATA(e));
    if(mv == NULL) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "SEND cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
    }
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "packet received");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "received packet", mv);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(SM_EVENT_DATA(ep), json_string);
        SM_EVENT_PRIORITY(ep)[0] = -ts.seconds;
        SM_EVENT_PRIORITY(ep)[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, COLLECTOR), &ep);
    }
    e->ctl.D = true;
    return 0;   
}

int send(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp();
    JSON_Value *mv = json_parse_string((char *)SM_EVENT_DATA((sm_event *)SM_STATE_EVENT_TRACE(s))); 
    if(mv == NULL) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "SEND cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "SEND cannot read the message reporting flag", "Malformed message JSON");
        result = DEFAULT_REPORTING_FLAG;
    }
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "packet sent");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "packet to send", mv);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(SM_EVENT_DATA(ep), json_string);
        SM_EVENT_PRIORITY(ep)[0] = -ts.seconds;
        SM_EVENT_PRIORITY(ep)[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, COLLECTOR), &ep);
    }
    sm_event *et = sm_state_pop_event(s);
    et->ctl.D = false;
    sm_lock_enqueue2(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, ROUTER_QUEUE), &et);
    //SM_STATE_EVENT_TRACE(s) = NULL;
    return 0;   
}

int rout(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    char *routing_status;

    void **t;
    sm_timestamp ts = sm_get_timestamp();
    JSON_Value *mv = json_parse_string((char *)SM_EVENT_DATA(e)); 
    if(mv == NULL) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "ROUT cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "ROUT cannot read the message reporting flag", 
                                       "Malformed message JSON");
        result = DEFAULT_REPORTING_FLAG;
    }
    if(json_object_has_value(m, "to") && json_object_get_string_len(m, "to") > 0) {
        const char *to = json_object_get_string(m, "to");
        t = sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, to);
        if(t == NULL) {
            SM_SYSLOG_CAUSE(SM_CORE, SM_LOG_ERR, "ROUT cannot find destination in the system directory", 
                                           "sm_directory_get_ref() returned NULL");
            routing_status = "failure";
            e->event_id = 2051;
        }
        else { // Routing assuming all 'to' fields point to tx-s
            e->ctl.D = false;
            e->event_id = 1;
            sm_lock_enqueue2(*(*(sm_tx **)t)->input_queue, &e);
            routing_status = "success";
        }
    }
    else {
        SM_SYSLOG_CAUSE(SM_CORE, SM_LOG_ERR, "ROUT cannot read destination address", 
                                       "json_object... returned NULL or zero length string");
        routing_status = "failure";
        e->event_id = 2051;
    }
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "packet routed");
        json_object_set_string(dtmp_o, "status", routing_status);
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "packet", mv);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(SM_EVENT_DATA(ep), json_string);
        SM_EVENT_PRIORITY(ep)[0] = -ts.seconds;
        SM_EVENT_PRIORITY(ep)[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, COLLECTOR), &ep);
    }
    return 0;   
}

int orig(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)SM_EVENT_DATA(e));
    if(mv == NULL) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "ORIG cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "SEND cannot read the message reporting flag", "Malformed message JSON");
        result = DEFAULT_REPORTING_FLAG;
    }
    JSON_Value *dtmp_v = json_value_init_object();
    JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
    JSON_Value *serv_v = json_value_deep_copy(json_object_dotget_value(m, "data.data"));
    JSON_Value *ip_v = json_value_init_object();
    JSON_Object *ip_o = json_value_get_object(ip_v);
    json_object_set_string(ip_o, "proto", "ip");
    json_object_set_string(ip_o, "from", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
    json_object_set_string(ip_o, "to", json_object_dotget_string(m, "data.to"));
    json_object_set_boolean(ip_o, "to_report", 1);
    json_object_set_value(ip_o, "data", serv_v);
    json_string = json_serialize_to_string_pretty(ip_v);
    sm_event *eo = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
    strcpy(SM_EVENT_DATA(eo), json_string);
    eo->event_id = 1;
    SM_STATE_EVENT_TRACE(s) = eo;
    if(result) { // to_report
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "packet originated");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "incoming packet", mv);
        json_object_set_value(dtmp_o, "originated packet", ip_v);
        json_string = json_serialize_to_string_pretty(dtmp_v);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
        strcpy(SM_EVENT_DATA(ep), json_string);
        ep->event_id = 1;
        SM_EVENT_PRIORITY(ep)[0] = -ts.seconds;
        SM_EVENT_PRIORITY(ep)[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, COLLECTOR), &ep);
    }
    return 0;   
}

int ssel(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)SM_EVENT_DATA(e));
    if(mv == NULL) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "SSEL cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv); 
    const char *proto;
    if(json_object_dothas_value(m, "data.proto")) {
        proto = json_object_dotget_string(m, "data.proto");
    }
    else {
        proto = json_object_dotget_string(m, "data.proto");
    }
    size_t i;
    for (i = 0; i < NUM_OF_PROTOCOLS; i++) {
        if(strcmp(protocols[i], proto) == 0) {
            e->event_id = i;
            break;
        }
    }
    const char *to = json_object_get_string(m, "to");
    if(to == NULL) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "SSEL cannot read the message to field", "Malformed message JSON");
        result = DEFAULT_REPORTING_FLAG;
    }
/*    if(i == 2 && ((strcmp(to, sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s)))) == 0) || 
                  (strcmp(to, sm_directory_get_name((SM_STATE_TX(s))->exec->dir, *(SM_STATE_TX(s))->input_queue)) == 0))) {
        i = 1;
    }*/
    if(i == NUM_OF_PROTOCOLS){
        e->event_id = 0;
    }
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "SSEL cannot read the message reporting flag", "Malformed message JSON");
        result = DEFAULT_REPORTING_FLAG;
    }
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "service selected");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_string(dtmp_o, "detected_proto", protocols[i]);
        json_object_set_number(dtmp_o, "assigned_event_id", e->event_id);
        json_object_set_value(dtmp_o, "packet", mv);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
        json_string = json_serialize_to_string_pretty(/*ip_v*/dtmp_v);
        strcpy(SM_EVENT_DATA(ep), json_string);
        SM_EVENT_PRIORITY(ep)[0] = -ts.seconds;
        SM_EVENT_PRIORITY(ep)[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, COLLECTOR), &ep);
    }
    return 0;
}

int dtmp(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)SM_EVENT_DATA(e));
    if(mv == NULL) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "DTMP cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    const char *command = json_object_dotget_string(m, "data.command");
    size_t i;
    for (i = 0; i < NUM_OF_COMMANDS; i++) {
        if(strcmp(commands[i], command) == 0) {
            e->event_id = i + PROTO_SPACE;
            break;
        }
    }
    if(i == NUM_OF_COMMANDS) {
        e->event_id = PROTO_SPACE;
    }
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "DTMP cannot read the message reporting flag", "Malformed message JSON");
        result = DEFAULT_REPORTING_FLAG;
    }
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "dtmp command selected");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_string(dtmp_o, "detected_command", commands[i]);
        json_object_set_number(dtmp_o, "assigned_event_id", e->event_id);
        json_object_set_value(dtmp_o, "packet", mv);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
        json_string = json_serialize_to_string_pretty(/*ip_v*/dtmp_v);
        strcpy(SM_EVENT_DATA(ep), json_string);
        SM_EVENT_PRIORITY(ep)[0] = -ts.seconds;
        SM_EVENT_PRIORITY(ep)[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, COLLECTOR), &ep);
    }
    return 0;
}

int appl(sm_event *e, sm_state *s) {
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *dtmp_v = json_value_init_object();
    JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
    json_object_set_string(dtmp_o, "reporter", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
    json_object_set_string(dtmp_o, "proto", "dtmp");
    json_object_set_string(dtmp_o, "command", "report");
    json_object_set_string(dtmp_o, "event", "event applied to SM");
    json_object_set_string(dtmp_o, "timestamp", ts.timestring);
    json_object_set_number(dtmp_o, "event_id", e->event_id);
    json_object_set_number(dtmp_o, "state_id", s->state_id);
    sm_fsm_node *n = sm_fsm_get_node(s);
    json_object_set_string(dtmp_o, "node_type", sm_node_type_to_string[n->type]);    
    sm_fsm_transition *t = sm_fsm_get_transition(e, s);
    json_object_set_string(dtmp_o, "transition_type", sm_transition_type_to_string[t->type]);    
    json_object_set_string(dtmp_o, "transition_ref", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, *(void **)t->transitionRef));
    json_object_set_number(dtmp_o, "target_node", t->targetNode);
    json_object_set_string(dtmp_o, "fsm_name", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, *(s->fsm)));
    JSON_Value *mv = json_parse_string((char *)SM_EVENT_DATA(e));
    if(mv == NULL) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "APPLY cannot parse message id", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    json_object_dotset_value(dtmp_o, "packet", mv);
    sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
    char *json_string = json_serialize_to_string_pretty(dtmp_v);
    strcpy(SM_EVENT_DATA(ep), json_string);
    SM_EVENT_PRIORITY(ep)[0] = -ts.seconds;
    SM_EVENT_PRIORITY(ep)[1] = -ts.nanoseconds;
    sm_pqueue_enqueue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, COLLECTOR), &ep);
    return 0;
}
/*
int drop(sm_event *e, sm_state *s) {
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *dtmp_v = json_value_init_object();
    JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
    json_object_set_string(dtmp_o, "reporter", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
    json_object_set_string(dtmp_o, "proto", "dtmp");
    json_object_set_string(dtmp_o, "command", "report");
    json_object_set_string(dtmp_o, "event", "closing event processing");
    json_object_set_string(dtmp_o, "timestamp", ts.timestring);
    json_object_set_number(dtmp_o, "event_id", e->event_id);
    json_object_set_number(dtmp_o, "state_id", s->state_id);
    sm_fsm_node *n = sm_fsm_get_node(s);
    json_object_set_string(dtmp_o, "node_type", sm_node_type_to_string[n->type]);    
    sm_fsm_transition *t = sm_fsm_get_transition(e, s);
    json_object_set_string(dtmp_o, "transition_type", sm_transition_type_to_string[t->type]);    
    json_object_set_string(dtmp_o, "transition_ref", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, *(void **)t->transitionRef));
    json_object_set_number(dtmp_o, "target_node", t->targetNode);
    json_object_set_string(dtmp_o, "fsm_name", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, *(s->fsm)));
    JSON_Value *mv = json_parse_string((char *)SM_EVENT_DATA(e));
    if(mv == NULL) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "DROP cannot parse message id", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    json_object_dotset_value(dtmp_o, "packet", mv);
    sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
    char *json_string = json_serialize_to_string_pretty(dtmp_v);
    strcpy(SM_EVENT_DATA(ep), json_string);
    SM_EVENT_PRIORITY(ep)[0] = -ts.seconds;
    SM_EVENT_PRIORITY(ep)[1] = -ts.nanoseconds;
    sm_pqueue_enqueue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, COLLECTOR), &ep);
    return 0;
}

int wait(sm_event *e, sm_state *s) { // DEPRECATED
    struct timespec ts = {0};
    ts.tv_sec = SM_EVENT_PRIORITY(e)[0];
    ts.tv_nsec = SM_EVENT_PRIORITY(e)[1];
    nanosleep(&ts, NULL);
    return 0;
}
*/
int noap(sm_event *e, sm_state *s) {
    return 0;
}

int serv(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)SM_EVENT_DATA(e));
    if(mv == NULL) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "SERV cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "SERV cannot read the message reporting flag", "Malformed message JSON");
        result = DEFAULT_REPORTING_FLAG;
    }
    JSON_Value *serv_v = json_value_init_object();
    JSON_Object *serv_o = json_value_get_object(serv_v);
    json_object_set_string(serv_o, "proto", "serv");
    json_object_set_number(serv_o, "answer", 42);
    JSON_Value *ip_v = json_value_init_object();
    JSON_Object *ip_o = json_value_get_object(ip_v);
    json_object_set_string(ip_o, "to", json_object_get_string(m, "from"));
    json_object_set_string(ip_o, "from", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
    json_object_set_string(ip_o, "proto", "ip");
    json_object_set_boolean(ip_o, "to_report", 1);
    json_object_set_value(ip_o, "data", serv_v);
    sm_event *ea = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
    json_string = json_serialize_to_string_pretty(ip_v);
    strcpy(SM_EVENT_DATA(ea), json_string);
    ea->event_id = 1;
    SM_STATE_EVENT_TRACE(s) = ea;
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "SERV request received");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "request packet", mv);
        json_object_set_value(dtmp_o, "response packet", ip_v);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(SM_EVENT_DATA(ep), json_string);
        SM_EVENT_PRIORITY(ep)[0] = -ts.seconds;
        SM_EVENT_PRIORITY(ep)[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, COLLECTOR), &ep);
    }
    return 0;
}    
/*
int fltr(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)SM_EVENT_DATA(e));
    if(mv == NULL) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "FLTR cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "FLTR cannot read the message reporting flag", "Malformed message JSON");
        result = DEFAULT_REPORTING_FLAG;
    }
    //size_t k_length = 0;
    char *k_position = (char *)SM_EVENT_HASH_KEY(e)->string;
    strcpy((char *)k_position, json_object_get_string(m, "from"));
    k_position += json_object_get_string_len(m, "from");
    strcpy((char *)k_position++, "#");
    //((char *)k_position++)[0] = "#";
    strcpy((char *)k_position, json_object_get_string(m, "to"));
    k_position += json_object_get_string_len(m, "to");
    strcpy((char *)k_position++, "#");
    strcpy((char *)k_position, json_object_get_string(m, "proto"));
    k_position += json_object_get_string_len(m, "proto");
    strcpy((char *)k_position, "\0");
    SM_EVENT_HASH_KEY(e)->length = strlen(SM_EVENT_HASH_KEY(e)->string);
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "flow filter key prepared");
        json_object_set_string(dtmp_o, "key", SM_EVENT_HASH_KEY(e)->string);
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "packet", mv);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(SM_EVENT_DATA(ep), json_string);
        SM_EVENT_PRIORITY(ep)[0] = -ts.seconds;
        SM_EVENT_PRIORITY(ep)[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, COLLECTOR), &ep);
    }
    return 0;
}

int setk(sm_event *e, sm_state *s) {
    //char error[80];
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)SM_EVENT_DATA(e));
    if(mv == NULL) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "SETK cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "SETK cannot read the message reporting flag", "Malformed message JSON");
        result = DEFAULT_REPORTING_FLAG;
    }
    size_t k_length = 0;
    char *k_position = NULL;
    switch(e->event_id) {
        case 1: // IP request_flow - from router to user
        case 4: // RTSP
            k_length += json_object_get_string_len(m, "from");
            k_length += json_object_get_string_len(m, "to");
            k_length += json_object_dotget_string_len(m, "data.proto");
            k_length += 3;
            SM_EVENT_HASH_KEY(e)->string = malloc(k_length);
            k_position = (char *)SM_EVENT_HASH_KEY(e)->string;
            strcpy(k_position, json_object_get_string(m, "from"));
            k_position += json_object_get_string_len(m, "from");
            strcpy(k_position++, "#");
            strcpy(k_position, json_object_get_string(m, "to"));
            k_position += json_object_get_string_len(m, "to");
            strcpy(k_position++, "#");
            strcpy(k_position, json_object_dotget_string(m, "data.proto"));
            k_position += json_object_dotget_string_len(m, "data.proto");
            strcpy(k_position, "\0");
            break;
        case 3: // SIP Call-ID
        case 1033: // DTMP reseipt indexed by SIP Call-ID
            SM_EVENT_HASH_KEY(e)->string = malloc(json_object_dotget_string_len(m, "data.Call-ID"));
            k_position = (char *)SM_EVENT_HASH_KEY(e)->string;
            strcpy((char *)k_position, json_object_dotget_string(m, "data.Call-ID"));
            break;
        case 1028: // set_flow - from user to router
            k_length += json_object_dotget_string_len(m, "data.data.from");
            k_length += json_object_dotget_string_len(m, "data.data.to");
            k_length += json_object_dotget_string_len(m, "data.data.proto");
            k_length += 3;
            SM_EVENT_HASH_KEY(e)->string = malloc(k_length);
            k_position = (char *)SM_EVENT_HASH_KEY(e)->string;
            strcpy(k_position, json_object_dotget_string(m, "data.data.from"));
            k_position += json_object_dotget_string_len(m, "data.data.from");
            strcpy(k_position++, "#");
            strcpy(k_position, json_object_dotget_string(m, "data.data.to"));
            k_position += json_object_dotget_string_len(m, "data.data.to");
            strcpy(k_position++, "#");
            strcpy(k_position, json_object_dotget_string(m, "data.data.proto"));
            k_position += json_object_dotget_string_len(m, "data.data.proto");
            strcpy(k_position, "\0");
            break;
        case 1030: // set_rule - from user to controller
        case 1031: // get_rule - from router to controller
        case 1032: // deploy_rule - from controller to router
            k_length += json_object_dotget_string_len(m, "data.data.from_role");
            k_length += json_object_dotget_string_len(m, "data.data.to_role");
            k_length += json_object_dotget_string_len(m, "data.data.proto");
            k_length += 2;
            SM_EVENT_HASH_KEY(e)->string = malloc(k_length);
            k_position = (char *)SM_EVENT_HASH_KEY(e)->string;
            strcpy(k_position, json_object_dotget_string(m, "data.data.from_role"));
            k_position += json_object_dotget_string_len(m, "data.data.from_role");
            strcpy(k_position++, "#");
            strcpy(k_position, json_object_dotget_string(m, "data.data.to_role"));
            k_position += json_object_dotget_string_len(m, "data.data.to_role");
            strcpy(k_position++, "#");
            strcpy(k_position, json_object_dotget_string(m, "data.data.proto"));
            k_position += json_object_dotget_string_len(m, "data.data.proto");
            strcpy(k_position, "\0");
            break;
        default: // Undefined
            SM_EVENT_HASH_KEY(e)->string = malloc(strlen("undefined"));
            strcpy(SM_EVENT_HASH_KEY(e)->string, "undefined"); *//*
            k_length += json_object_get_string_len(m, "from");
            k_length += json_object_get_string_len(m, "to");
            k_length += json_object_dotget_string_len(m, "data.proto");
            k_length += 3;
            SM_EVENT_HASH_KEY(e)->string = malloc(k_length);
            k_position = (char *)SM_EVENT_HASH_KEY(e)->string;
            strcpy(k_position, json_object_get_string(m, "from"));
            k_position += json_object_get_string_len(m, "from");
            strcpy(k_position++, "#");
            strcpy(k_position, json_object_get_string(m, "to"));
            k_position += json_object_get_string_len(m, "to");
            strcpy(k_position++, "#");
            strcpy(k_position, json_object_dotget_string(m, "data.proto"));
            k_position += json_object_dotget_string_len(m, "data.proto");
            strcpy(k_position, "\0"); *//*
            break;
            //sprintf(error, "SETK cannot conpose key for event-Id %ld in state with state-Id %ld", e->event_id, s->state_id);
            //SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, error, "Malformed message JSON");
            //usleep(100);
            //return EXIT_FAILURE;
            //break;
    }
    SM_EVENT_HASH_KEY(e)->length = strlen(SM_EVENT_HASH_KEY(e)->string);
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "flow filter key prepared");
        json_object_set_string(dtmp_o, "key", SM_EVENT_HASH_KEY(e)->string);
        json_object_set_number(dtmp_o, "eventId", e->event_id);
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "packet", mv);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(SM_EVENT_DATA(ep), json_string);
        SM_EVENT_PRIORITY(ep)[0] = -ts.seconds;
        SM_EVENT_PRIORITY(ep)[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, COLLECTOR), &ep);
    }
    return 0;
}

int setf(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)SM_EVENT_DATA(e));
    if(mv == NULL) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "SETF cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "SETF cannot read the message reporting flag", "Malformed message JSON");
        result = DEFAULT_REPORTING_FLAG;
    }
    strcpy(SM_STATE_DATA(s), json_object_dotget_string(m, "data.data.decision"));
    if(json_object_has_value(m, "data.data.from_role")) {
        char* from = malloc(json_object_dotget_string_len(m, "data.data.from") + 1);
        char* from_role = malloc(json_object_dotget_string_len(m, "data.data.from_role"));
        from[0] = '$';
        strcpy(from + 1, json_object_dotget_string(m, "data.data.from"));
        strcpy(from_role, json_object_dotget_string(m, "data.data.from_role"));
        //const char* from = json_object_dotget_string(m, "data.data.from");
        sm_directory_set((SM_STATE_TX(s))->exec->dir, (const char *)from, from_role);
    }
    if(json_object_has_value(m, "data.data.to_role")) {
        char* to = malloc(json_object_dotget_string_len(m, "data.data.to") + 1);
        char* to_role = malloc(json_object_dotget_string_len(m, "data.data.to_role"));
        to[0] = '$';
        strcpy(to + 1, json_object_dotget_string(m, "data.data.to"));
        strcpy(to_role, json_object_dotget_string(m, "data.data.to_role"));
        //const char* to = json_object_dotget_string(m, "data.data.to");
        sm_directory_set((SM_STATE_TX(s))->exec->dir, (const char *) to, to_role);
    }
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "flow was set up");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "request packet", mv);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(SM_EVENT_DATA(ep), json_string);
        SM_EVENT_PRIORITY(ep)[0] = -ts.seconds;
        SM_EVENT_PRIORITY(ep)[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, COLLECTOR), &ep);
    }
    return 0;
}

int reqf(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)SM_EVENT_DATA(e));
    if(mv == NULL) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "REQF cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "REQF cannot read the message reporting flag", "Malformed message JSON");
        result = DEFAULT_REPORTING_FLAG;
    }
    JSON_Value *req_v = json_value_init_object();
    JSON_Object *req_o = json_value_get_object(req_v);
    json_object_set_string(req_o, "proto", "dtmp");
    json_object_set_string(req_o, "command", "request_flow");
    json_object_dotset_string(req_o, "data.from", json_object_get_string(m, "from"));
    json_object_dotset_string(req_o, "data.to", json_object_get_string(m, "to"));
    json_object_dotset_string(req_o, "data.proto", json_object_dotget_string(m, "data.proto"));
    json_object_dotset_string(req_o, "data.key", SM_EVENT_HASH_KEY(e)->string);
    JSON_Value *ip_v = json_value_init_object();
    JSON_Object *ip_o = json_value_get_object(ip_v);
    json_object_set_string(ip_o, "to", CLIENT);
    json_object_set_string(ip_o, "from", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
    json_object_set_string(ip_o, "proto", "ip");
    json_object_set_boolean(ip_o, "to_report", 1);
    json_object_set_value(ip_o, "data", req_v);
    json_string = json_serialize_to_string_pretty(ip_v);
    sm_event *er = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
    strcpy(SM_EVENT_DATA(er), json_string);
    er->event_id = 1; 
    SM_STATE_EVENT_TRACE(s) = er;
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "flow requested from user");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "processed packet", mv);
        json_object_set_value(dtmp_o, "request packet", ip_v);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(SM_EVENT_DATA(ep), json_string);
        SM_EVENT_PRIORITY(ep)[0] = -ts.seconds;
        SM_EVENT_PRIORITY(ep)[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, COLLECTOR), &ep);
    }
    return 0;
}    

int reqr(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)SM_EVENT_DATA(e));
    if(mv == NULL) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "REQR cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "REQR cannot read the message reporting flag", "Malformed message JSON");
        result = DEFAULT_REPORTING_FLAG;
    }
    JSON_Value *req_v = json_value_init_object();
    JSON_Object *req_o = json_value_get_object(req_v);

    char *key = malloc(json_object_get_string_len(m, "from") + 1);
    key[0] = '$';
    strcpy(key + 1, json_object_get_string(m, "from"));
    void **tmp = sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, key);
    char *from_role = tmp?(char *)(*tmp):NULL;
    key = malloc(json_object_get_string_len(m, "to") + 1);
    key[0] = '$';
    strcpy(key + 1, json_object_get_string(m, "to"));
    tmp = sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, key);
    char *to_role = tmp?(char *)(*tmp):NULL;
    free(key);

    JSON_Value *ip_v = json_value_init_object();
    JSON_Object *ip_o = json_value_get_object(ip_v);
    if(from_role  == NULL || to_role == NULL ) {
        e->event_id = 2051;
    }
    else {
        json_object_set_string(req_o, "proto", "dtmp");
        json_object_set_string(req_o, "command", "get_rule");
        json_object_dotset_string(req_o, "data.key", (char *)SM_EVENT_HASH_KEY(e)->string);
        json_object_dotset_string(req_o, "data.from_role", from_role);
        json_object_dotset_string(req_o, "data.to_role", to_role);
        json_object_dotset_string(req_o, "data.key", SM_EVENT_HASH_KEY(e)->string);
        json_object_dotset_string(req_o, "data.proto", json_object_get_string(m, "proto"));
        json_object_set_string(ip_o, "to", CONTROLLER);
        json_object_set_string(ip_o, "from", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
        json_object_set_string(ip_o, "proto", "ip");
        json_object_set_boolean(ip_o, "to_report", 1);
        json_object_set_value(ip_o, "data", req_v);
        json_string = json_serialize_to_string_pretty(ip_v);
        sm_event *er = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
        strcpy(SM_EVENT_DATA(er), json_string);
        er->event_id = 1; 
        SM_STATE_EVENT_TRACE(s) = er;
        e->event_id = 1031;
    }
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "rule requested by router");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "processed packet", mv);
        json_object_set_value(dtmp_o, "request packet", ip_v);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(SM_EVENT_DATA(ep), json_string);
        SM_EVENT_PRIORITY(ep)[0] = -ts.seconds;
        SM_EVENT_PRIORITY(ep)[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, COLLECTOR), &ep);
    }
    return 0;
}    

int chck(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)SM_EVENT_DATA(e));
    if(mv == NULL) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "CHCK cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv); 
    if(strcmp(SM_STATE_DATA(s), "allowed") == 0) {
        e->event_id = 2050;
    }
    else if (strcmp(SM_STATE_DATA(s), "blocked") == 0)
    {
        e->event_id = 2049;
    }
    else {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "CHCK cannot parse decision", "Malformed decision value in s->data");
        return EXIT_FAILURE;
    }
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "CHCK cannot read the message reporting flag", "Malformed message JSON");
        result = DEFAULT_REPORTING_FLAG;
    }
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "flow checked");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_string(dtmp_o, "flow decision", SM_STATE_DATA(s));
        json_object_set_string(dtmp_o, "key", SM_EVENT_HASH_KEY(e)->string);
        json_object_set_value(dtmp_o, "packet", mv);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(SM_EVENT_DATA(ep), json_string);
        SM_EVENT_PRIORITY(ep)[0] = -ts.seconds;
        SM_EVENT_PRIORITY(ep)[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, COLLECTOR), &ep);
    }
    return 0;
}

int getr(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)SM_EVENT_DATA(e));
    if(mv == NULL) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "GETR cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "GETR cannot read the message reporting flag", "Malformed message JSON");
        result = DEFAULT_REPORTING_FLAG;
    }
    json_object_dotset_string(m, "data.data.decision", (char *)SM_STATE_DATA(s));
    json_string = json_serialize_to_string_pretty(mv);
    strcpy(SM_EVENT_DATA(e), json_string);
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "GETR request received");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "packet", mv);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(SM_EVENT_DATA(ep), json_string);
        SM_EVENT_PRIORITY(ep)[0] = -ts.seconds;
        SM_EVENT_PRIORITY(ep)[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, COLLECTOR), &ep);
    }
    return 0;
}    

int wrrt(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)SM_EVENT_DATA(e));
    if(mv == NULL) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "WRRT cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "WRRT cannot read the message reporting flag", "Malformed message JSON");
        result = DEFAULT_REPORTING_FLAG;
    }
    JSON_Value *getr_v = json_value_init_object();
    JSON_Object *getr_o = json_value_get_object(getr_v);
    json_object_set_string(getr_o, "proto", "dtmp");
    json_object_set_string(getr_o, "command", "deploy_rule");
    json_object_dotset_string(getr_o, "data.to_role", json_object_dotget_string(m, "data.data.to_role"));
    json_object_dotset_string(getr_o, "data.from_role", json_object_dotget_string(m, "data.data.from_role"));
    json_object_dotset_string(getr_o, "data.proto", json_object_dotget_string(m, "data.data.proto"));
    json_object_dotset_string(getr_o, "data.key", json_object_dotget_string(m, "data.data.key"));
    json_object_dotset_string(getr_o, "data.decision", json_object_dotget_string(m, "data.data.decision"));
    JSON_Value *ip_v = json_value_init_object();
    JSON_Object *ip_o = json_value_get_object(ip_v);
    json_object_set_string(ip_o, "to", json_object_get_string(m, "from"));
    json_object_set_string(ip_o, "from", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
    json_object_set_string(ip_o, "proto", "ip");
    json_object_set_boolean(ip_o, "to_report", 1);
    json_object_set_value(ip_o, "data", getr_v);
    sm_event *ea = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
    json_string = json_serialize_to_string_pretty(ip_v);
    strcpy(SM_EVENT_DATA(ea), json_string);
    ea->event_id = 1;
    SM_STATE_EVENT_TRACE(s) = ea;
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "GETR response prepared received");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "request packet", mv);
        json_object_set_value(dtmp_o, "response packet", ip_v);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(SM_EVENT_DATA(ep), json_string);
        SM_EVENT_PRIORITY(ep)[0] = -ts.seconds;
        SM_EVENT_PRIORITY(ep)[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, COLLECTOR), &ep);
    }
    return 0;
}

int addr(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)SM_EVENT_DATA(e));
    if(mv == NULL) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "ADDR cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "ADDR cannot read the message reporting flag", "Malformed message JSON");
        result = DEFAULT_REPORTING_FLAG;
    }
    strcpy(SM_STATE_DATA(s), json_object_dotget_string(m, "data.data.decision"));
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "rule was set up");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "request packet", mv);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(SM_EVENT_DATA(ep), json_string);
        SM_EVENT_PRIORITY(ep)[0] = -ts.seconds;
        SM_EVENT_PRIORITY(ep)[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, COLLECTOR), &ep);
    }
    return 0;
}

int xtrr(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)SM_EVENT_DATA(e));
    if(mv == NULL) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "ADDR cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "ADDR cannot read the message reporting flag", "Malformed message JSON");
        result = DEFAULT_REPORTING_FLAG;
    }
    json_object_dotset_string(m, "data.direction", json_object_dotget_string(m, "data.rule.direction"));
    //strcpy(s->data, json_serialize_to-string_pretty(json_object_dotget_value(m, "data.rule")));
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "rule was set up");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "request packet", mv);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(SM_EVENT_DATA(ep), json_string);
        SM_EVENT_PRIORITY(ep)[0] = -ts.seconds;
        SM_EVENT_PRIORITY(ep)[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, COLLECTOR), &ep);
    }
    return 0;
}

int askf(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)SM_EVENT_DATA(e));
    if(mv == NULL) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "ASKR cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "ASKR cannot read the message reporting flag", "Malformed message JSON");
        result = DEFAULT_REPORTING_FLAG;
    }
    JSON_Value *getr_v = json_value_init_object();
    JSON_Object *getr_o = json_value_get_object(getr_v);
    json_object_set_string(getr_o, "proto", "dtmp");
    json_object_set_string(getr_o, "command", "set_flow");
    json_object_dotset_string(getr_o, "data.decision", "allowed");
    const char *to = json_object_dotget_string(m, "data.data.to");
    const char *from = json_object_dotget_string(m, "data.data.from");
    size_t i;
    char *to_role = NULL;
    char *from_role = NULL;
    for(i = 0; i < NUM_OF_END_NODES; i++) {
        if(strcmp(end_nodes[i], to) == 0) {
            to_role = roles[assigned_role[i]];
        }
        if(strcmp(end_nodes[i], from) == 0) {
            from_role = roles[assigned_role[i]];
        }
    }
    if(to_role != NULL) {
        json_object_dotset_string(getr_o, "data.to_role", to_role);
        json_object_dotset_string(m, "data.data.to_role", to_role);
    }
    if(from_role != NULL) {
        json_object_dotset_string(getr_o, "data.from_role", from_role);
        json_object_dotset_string(m, "data.data.from_role", from_role);
    }
    json_string = json_serialize_to_string_pretty(mv);
    strcpy(SM_EVENT_DATA(e), json_string);
    json_object_dotset_string(getr_o, "data.from", json_object_dotget_string(m, "data.data.from"));
    json_object_dotset_string(getr_o, "data.to", json_object_dotget_string(m, "data.data.to"));
    json_object_dotset_string(getr_o, "data.proto", json_object_dotget_string(m, "data.data.proto"));
    json_object_dotset_string(getr_o, "data.key", json_object_dotget_string(m, "data.data.key"));
    json_object_dotset_string(getr_o, "data.decision", "allowed");
    JSON_Value *ip_v = json_value_init_object();
    JSON_Object *ip_o = json_value_get_object(ip_v);
    json_object_set_string(ip_o, "to", json_object_get_string(m, "from"));
    json_object_set_string(ip_o, "from", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
    json_object_set_string(ip_o, "proto", "ip");
    json_object_set_boolean(ip_o, "to_report", 1);
    json_object_set_value(ip_o, "data", getr_v);
    sm_event *ea = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
    json_string = json_serialize_to_string_pretty(ip_v);
    strcpy(SM_EVENT_DATA(ea), json_string);
    ea->event_id = 1;
    SM_STATE_EVENT_TRACE(s) = ea;
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "request_flow command received");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "request packet", mv);
        json_object_set_value(dtmp_o, "response packet", ip_v);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(SM_EVENT_DATA(ep), json_string);
        SM_EVENT_PRIORITY(ep)[0] = -ts.seconds;
        SM_EVENT_PRIORITY(ep)[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, COLLECTOR), &ep);
    }
    return 0;
}

int askr(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)SM_EVENT_DATA(e));
    if(mv == NULL) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "ASKR cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "ASKR cannot read the message reporting flag", "Malformed message JSON");
        result = DEFAULT_REPORTING_FLAG;
    }
    JSON_Value *getr_v = json_value_init_object();
    JSON_Object *getr_o = json_value_get_object(getr_v);
    json_object_set_string(getr_o, "proto", "dtmp");
    json_object_set_string(getr_o, "command", "set_rule");
    json_object_dotset_string(getr_o, "data.decision", "allowed");
    json_object_dotset_string(getr_o, "data.to_role", json_object_dotget_string(m, "data.data.to_role"));
    json_object_dotset_string(getr_o, "data.from_role", json_object_dotget_string(m, "data.data.from_role"));
    json_object_dotset_string(getr_o, "data.key", json_object_dotget_string(m, "data.data.key"));
    json_object_dotset_string(getr_o, "data.proto", json_object_dotget_string(m, "data.data.proto"));
    JSON_Value *ip_v = json_value_init_object();
    JSON_Object *ip_o = json_value_get_object(ip_v);
    json_object_set_string(ip_o, "to", CONTROLLER);
    json_object_set_string(ip_o, "from", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
    json_object_set_string(ip_o, "proto", "ip");
    json_object_set_boolean(ip_o, "to_report", 1);
    json_object_set_value(ip_o, "data", getr_v);
    sm_event *ea = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
    json_string = json_serialize_to_string_pretty(ip_v);
    strcpy(SM_EVENT_DATA(ea), json_string);
    ea->event_id = 1;
    SM_STATE_EVENT_TRACE(s) = ea;
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "rule sent after request flow command processing");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "request packet", mv);
        json_object_set_value(dtmp_o, "response packet", ip_v);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(SM_EVENT_DATA(ep), json_string);
        SM_EVENT_PRIORITY(ep)[0] = -ts.seconds;
        SM_EVENT_PRIORITY(ep)[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, COLLECTOR), &ep);
    }
    return 0;
}    

int s100(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)SM_EVENT_DATA(e));
    if(mv == NULL) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "s100 cannot parse message", "malformed message json");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "s100 cannot read the message reporting flag", "malformed message json");
        result = DEFAULT_REPORTING_FLAG;
    }
    JSON_Value *s100_v = json_value_init_object();
    JSON_Object *s100_o = json_value_get_object(s100_v);
    json_object_set_string(s100_o, "proto", "sip");
    //json_object_set_string(s100_o, "", json_object_dotget_string(""));
    json_object_dotset_string(s100_o, "data.request-line.method", "100 Trying");
    json_object_dotset_string(s100_o, "data.request-line.version", "sip/2.0");
    json_object_dotset_string(s100_o, "data.via", json_object_dotget_string(m, "data.via"));
    json_object_dotset_string(s100_o, "data.from", json_object_dotget_string(m, "data.from"));
    json_object_dotset_string(s100_o, "data.to", json_object_dotget_string(m, "data.to"));
    json_object_dotset_string(s100_o, "data.call-id", json_object_dotget_string(m, "data.call-id"));
    json_object_dotset_string(s100_o, "data.cseq", json_object_dotget_string(m, "data.cseq"));
    JSON_Value *ip_v = json_value_init_object();
    JSON_Object *ip_o = json_value_get_object(ip_v);
    json_object_set_string(ip_o, "to", json_object_get_string(m, "from"));
    json_object_set_string(ip_o, "from", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
    json_object_set_string(ip_o, "proto", "ip");
    json_object_set_boolean(ip_o, "to_report", 1);
    json_object_set_value(ip_o, "data", s100_v);
    sm_event *ea = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
    json_string = json_serialize_to_string_pretty(ip_v);
    strcpy(SM_EVENT_DATA(ea), json_string);
    ea->event_id = 1;
    SM_STATE_EVENT_TRACE(s) = ea;
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "s100 request received");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "request packet", mv);
        json_object_set_value(dtmp_o, "response packet", ip_v);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(SM_EVENT_DATA(ep), json_string);
        SM_EVENT_PRIORITY(ep)[0] = -ts.seconds;
        SM_EVENT_PRIORITY(ep)[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, COLLECTOR), &ep);
    }
    return 0;
}

int sinv(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)SM_EVENT_DATA(e));
    if(mv == NULL) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "S100 cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "S100 cannot read the message reporting flag", "Malformed message JSON");
        result = DEFAULT_REPORTING_FLAG;
    }
    JSON_Value *sinv_v = json_value_init_object();
    JSON_Object *sinv_o = json_value_get_object(sinv_v);
    json_object_set_string(sinv_o, "proto", "sip");
    //json_object_set_string(sinv_o, "", json_object_dotget_string(""));
    json_object_dotset_string(sinv_o, "data.Request-Line.Method", json_object_dotget_string(m, "data.Request-Line.Method"));
    json_object_dotset_string(sinv_o, "data.Request-Line.Request-URI", json_object_dotget_string(m, "data.Request-Line.Request-URI"));
    json_object_dotset_string(sinv_o, "data.Request-Line.Version", json_object_dotget_string(m, "data.Request-Line.Version"));
    json_object_dotset_string(sinv_o, "data.Via", json_object_dotget_string(m, "data.Via"));
    json_object_dotset_string(sinv_o, "data.From", json_object_dotget_string(m, "data.From"));
    json_object_dotset_string(sinv_o, "data.To", json_object_dotget_string(m, "data.To"));
    json_object_dotset_string(sinv_o, "data.Call-ID", json_object_dotget_string(m, "data.Call-ID"));
    json_object_dotset_number(sinv_o, "data.Max-Forwards", json_object_dotget_number(m, "data.Max-Forwards") - 1);
    json_object_dotset_string(sinv_o, "data.CSeq", json_object_dotget_string(m, "data.CSeq"));
    JSON_Value *ip_v = json_value_init_object();
    JSON_Object *ip_o = json_value_get_object(ip_v);
    json_object_set_string(ip_o, "to", json_object_get_string(m, "from"));
    json_object_set_string(ip_o, "from", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
    json_object_set_string(ip_o, "proto", "ip");
    json_object_set_boolean(ip_o, "to_report", 1);
    json_object_set_value(ip_o, "data", sinv_v);
    sm_event *ea = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
    json_string = json_serialize_to_string_pretty(ip_v);
    strcpy(SM_EVENT_DATA(ea), json_string);
    ea->event_id = 1;
    SM_STATE_EVENT_TRACE(s) = ea;
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "INVITE is forwarded");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "request packet", mv);
        json_object_set_value(dtmp_o, "response packet", ip_v);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(SM_EVENT_DATA(ep), json_string);
        SM_EVENT_PRIORITY(ep)[0] = -ts.seconds;
        SM_EVENT_PRIORITY(ep)[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, COLLECTOR), &ep);
    }
    return 0;
}

int sips(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)SM_EVENT_DATA(e));
    if(mv == NULL) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "SIPS cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    const char *command = json_object_dotget_string(m, "data.Request-Line.Method");
    size_t i;
    for (i = 0; i < SIP_NUM_OF_METHODS; i++) {
        if(strcmp(sip_methods[i], command) == 0) {
            e->event_id = i + PROTO_SPACE;
            break;
        }
    }
    if(i == SIP_NUM_OF_METHODS) {
        e->event_id = PROTO_SPACE + COMMAND_SPACE + STATUS_SPACE;
    }
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG_CAUSE(SM_JSON, SM_LOG_ERR, "SIPS cannot read the message reporting flag", "Malformed message JSON");
        result = DEFAULT_REPORTING_FLAG;
    }
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name((SM_STATE_TX(s))->exec->dir, (SM_STATE_TX(s))));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "sip method selected");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_string(dtmp_o, "command", commands[i]);
        json_object_set_number(dtmp_o, "assigned_event_id", e->event_id);
        json_object_set_value(dtmp_o, "packet", mv);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, POOL));
        json_string = json_serialize_to_string_pretty(*//*ip_v*//*dtmp_v);
        strcpy(SM_EVENT_DATA(ep), json_string);
        SM_EVENT_PRIORITY(ep)[0] = -ts.seconds;
        SM_EVENT_PRIORITY(ep)[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(*sm_directory_get_ref((SM_STATE_TX(s))->exec->dir, COLLECTOR), &ep);
    }
    return 0;
}*/

