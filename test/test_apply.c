/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Apply function tests
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "test_utils.h"
//#include "../src/sm.h"
#include "apply_apps.h"
#include "../lib/parson/parson.h"

#define APPLY_APPS "apply_apps.so"
#define APPLY_CLIENT_SM "client.json"
#define APPLY_ROUTER_SM "router.json"
#define APPLY_SERVER_SM "server.json"

extern char *protocols[];
extern char *commands[];

int main() {

    // Create the system directory
    sm_directory *dir = sm_directory_create();

    // Create and register the executor object 
    sm_exec *exec = sm_exec_create(65536, dir);
    dir = sm_directory_set(dir, "exec", exec);
    // !! Rework this to remove this work-around!!
    exec->dir = dir;

    // Create and register the reporting priority queue
    sm_pqueue *pq = sm_pqueue_create(QUEUE_SIZE, true);
    dir = sm_directory_set(dir, COLLECTOR, pq);

    // Create and register the reporting priority queue
    // sm_pqueue *pq = sm_pqueue_create(64, true);
    // dir = sm_directory_set(dir, "col_pqueue", pq);     
    
    // Load and register the applications
    void *handle = dlopen(APPLY_APPS, RTLD_LAZY);
    
    // Report sending packet 
    sm_app send = dlsym(handle, "send");
    dir = sm_directory_set(dir, "SEND", send);
    
    // Report routing packet
    sm_app rout = dlsym(handle, "rout");
    dir = sm_directory_set(dir, "ROUT", rout);
    
    // Report receiving packet
    sm_app recv = dlsym(handle, "recv");
    dir = sm_directory_set(dir, "RECV", recv);
    
    // Command node to originate packet
    sm_app orig = dlsym(handle, "orig");
    dir = sm_directory_set(dir, "ORIG", orig);
    
    // Service selector
    sm_app ssel = dlsym(handle, "ssel");
    dir = sm_directory_set(dir, "SSEL", ssel);
    
    // DTMP command parser
    sm_app dtmp = dlsym(handle, "dtmp");
    dir = sm_directory_set(dir, "DTMP", dtmp);
    
    // Report event application to SM
    sm_app appl = dlsym(handle, "appl");
    dir = sm_directory_set(dir, "APPL", appl);
 /*   
    // Pause SM processing
    sm_app wait = dlsym(handle, "wait");
    dir = sm_directory_set(dir, "WAIT", wait);
 */   
    // No application
    sm_app noap = dlsym(handle, "send");
    dir = sm_directory_set(dir, "NOAP", noap);
    
    // Server dummy application
    sm_app serv = dlsym(handle, "serv");
    dir = sm_directory_set(dir, "SERV", serv);
    
    sm_print_directory(dir);

    // Compile and register the client SM
    FILE * file = fopen(APPLY_CLIENT_SM, "r");
    if (!file) {
        SM_REPORT_MESSAGE(SM_LOG_ERR, "Input Client SM file is missing or incorrect\n");
        exit(0);
    }
    char jstr[32 * 1024];
    int i=0;
    while(!feof(file))
        jstr[i++] = fgetc(file);
    jstr[--i]='\0';
    fclose(file);
    sm_fsm *client_sm = sm_fsm_create(jstr, dir);
    dir = sm_directory_set(dir, CLIENT_SM_NAME, client_sm);
    // printf("\nClient SM:\n%s\n", sm_fsm_to_string(client_sm, dir));
 
    // Compile and register the router SM
    file = fopen(APPLY_ROUTER_SM, "r");
    if (!file) {
        SM_REPORT_MESSAGE(SM_LOG_ERR, "Input Router SM file is missing or incorrect\n");
        exit(0);
    }
    i=0;
    while(!feof(file))
        jstr[i++] = fgetc(file);
    jstr[--i]='\0';
    fclose(file);
    sm_fsm *router_sm = sm_fsm_create(jstr, dir);
    dir = sm_directory_set(dir, ROUTER_SM_NAME, router_sm);
    // printf("\nRouter SM:\n%s\n", sm_fsm_to_string(router_sm, dir));
    
    // Compile and register the server SM
    file = fopen(APPLY_SERVER_SM, "r");
    if (!file) {
        SM_REPORT_MESSAGE(SM_LOG_ERR, "Input Server SM file is missing or incorrect\n");
        exit(0);
    }
    i=0;
    while(!feof(file))
        jstr[i++] = fgetc(file);
    jstr[--i]='\0';
    fclose(file);
    sm_fsm *server_sm = sm_fsm_create(jstr, dir);
    dir = sm_directory_set(dir, SERVER_SM_NAME, server_sm);
    
    // Create and register the worker bipriority queues q0 & q1
    sm_queue2 *q0 = sm_queue2_create();
    dir = sm_directory_set(dir, CLIENT_QUEUE, q0); 
    sm_queue2 *q1 = sm_queue2_create();
    dir = sm_directory_set(dir, SERVER_QUEUE, q1); 
    
    // Create and register the router bipriority queue q2
    sm_queue2 *q2 = sm_queue2_create();
    dir = sm_directory_set(dir, ROUTER_QUEUE, q2); 
    
    // Create and register the thread-worker tx0(exec, worker SM, q0, sync = true)
    sm_tx *tx0 = sm_tx_create(exec, (sm_fsm **)sm_directory_get_ref(dir, CLIENT_SM_NAME), 4096, 4096, 
                              (sm_queue2 **)sm_directory_get_ref(dir, CLIENT_QUEUE), true);
    dir = sm_directory_set(dir, CLIENT, tx0);  
    
    // Create and register the thread-worker tx1(exec, worker SM, q1, sync = true)
    sm_tx *tx1 = sm_tx_create(exec, (sm_fsm **)sm_directory_get_ref(dir, SERVER_SM_NAME), 4096, 4096, 
                              (sm_queue2 **)sm_directory_get_ref(dir, SERVER_QUEUE), true);
    dir = sm_directory_set(dir, SERVER, tx1);  
    
    // Create and register the thread-worker tx2(exec, router SM, q2, sync = true)i
    sm_tx *tx2 = sm_tx_create(exec, (sm_fsm **)sm_directory_get_ref(dir, ROUTER_SM_NAME), 4096, 4096, 
                              (sm_queue2 **)sm_directory_get_ref(dir, ROUTER_QUEUE), true);
    dir = sm_directory_set(dir, ROUTER, tx2);  
    
    // Create and register the event pool(16, 4k)
    sm_queue *p0 = sm_queue_create(4096, false, true, false, 32, true);
    dir = sm_directory_set(dir, POOL, p0);

    // Check directory
    //sm_print_directory(tx0->exec->dir);

    // Run threads:
    pthread_t t0;
    pthread_create(&t0, NULL, &sm_tx_runner, (void *)tx0);
    pthread_t t1;
    pthread_create(&t1, NULL, &sm_tx_runner, (void *)tx1);
    pthread_t t2;
    pthread_create(&t2, NULL, &sm_tx_runner, (void *)tx2);

    // Use-Case 1: request-responce with SERV protocol
    // ---
    // 1. Dequeue event from the pool
    sm_event *e = sm_queue_dequeue(p0);
        
    // 2. Compose the SERV request tx0->tx2  wrapped into the ORIG DTMP command
    JSON_Value *serv_v = json_value_init_object();
    JSON_Object *serv_o = json_value_get_object(serv_v);
    json_object_set_string(serv_o, "proto", "serv");
    json_object_set_string(serv_o, "command", "request");

    // 3. Wrap SERV into DTMP
    JSON_Value *dtmp_v = json_value_init_object();
    JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
    json_object_set_string(dtmp_o, "proto", "dtmp");
    json_object_set_string(dtmp_o, "command", "originate_packet");
    //json_object_set_string(dtmp_o, "to", SERVER_QUEUE);
    json_object_set_string(dtmp_o, "to", SERVER);
    json_object_set_value(dtmp_o, "data", serv_v);
    
    // 4. Wrap DTMP into IP
    JSON_Value *ip_v = json_value_init_object();
    JSON_Object *ip_o = json_value_get_object(ip_v);
    json_object_set_string(ip_o, "proto", "ip");
    json_object_set_string(ip_o, "from", CONTROLLER);
    //json_object_set_string(ip_o, "to", CLIENT_QUEUE);
    json_object_set_string(ip_o, "to", CLIENT);
    json_object_set_boolean(ip_o, "to_report", 1);
    json_object_set_value(ip_o, "data", dtmp_v);

    // 5. Serialize JSON object and copy to the event data block
    char *json_string = json_serialize_to_string_pretty(ip_v);
    strcpy(SM_EVENT_DATA(e), json_string);
    e->event_id = 1;

    // 6. Send the request to tx0
    sm_lock_enqueue2(*sm_directory_get_ref(dir, ROUTER_QUEUE), &e);

    // 7. Wait 1 sec.
    sleep(1);

    // 8. Print out the reporting queue
    sm_event *r;
    while (SM_PQUEUE_SIZE(pq) != 0)
    {
        r = sm_pqueue_dequeue(pq); 
        //sm_print_event(r);
        printf("\n%s", (char *)SM_EVENT_DATA(r));
    }

    // send kill event
    /*
    pthread_join(t0, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    */
    return 0;
}
