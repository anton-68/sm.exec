/* SM.EXEC
   FSM module tests
   anton.bondarenko@gmail.com */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/sm_sys.h"
#include "../../src/sm_fsm.h"
#include "../../src/sm_app.h"
#include "../../src/sm_event.h"
#include "../../oam/logger.h"


#define OUTPUT_BUF_LEN (1024 * 32)

/* fsm pretty print */

static char *to_string(sm_fsm* f) {
	if(f == NULL) 
		return "";
	char *output;
	if((output = malloc(OUTPUT_BUF_LEN)) == NULL) {
		printf("\nmalloc() failed");
		exit(0);
	}
	char line[1024];
	char *node_type[] = {"undefined", "state", "initial", "final", "joint"};
	sprintf(output, "FSM:\n");
	sprintf(line, "max number of node :  %lu\n", f->num_of_nodes - 1);
	strcat(output, line);
    sprintf(line, "max number of event :  %lu\n", f->num_of_events - 1);
	strcat(output, line);
	sprintf(line, "initial event Id :  %lu\n", f->initial);
	strcat(output, line);
	sprintf(line, "final event Id :  %lu\n", f->final);
	strcat(output, line);
	sprintf(line, "default event Id :  %lu\n", f->omega);
	strcat(output, line);
	sprintf(line, "nodes :\n");
	strcat(output, line);
	for(size_t i; i < f->num_of_nodes; i++){
		sprintf(line, "node Id: %lu, node type: %s\n", i, node_type[f->nodes[i]]);
		strcat(output, line);
	}
	sprintf(line, "transition function :\n");	
	strcat(output, line);
	for(size_t i = 0; i < f->num_of_nodes; i++){
		sprintf(line, "node Id: %lu, transitions:", i);
		strcat(output, line);
		for(size_t j = 0; j < f->num_of_events; j++){
			sprintf(line, " %lu", f->table[i][j].new_node);
			strcat(output, line);
		}
		sprintf(line, "\n");
		strcat(output, line);
	}
	sprintf(line, "output function (actions) :\n");	
	strcat(output, line);
	
	for(size_t i = 0; i < f->num_of_nodes; i++){
		sprintf(line, "node Id: %lu, actions:", i);
		strcat(output, line);
		for(size_t j = 0; j < f->num_of_events; j++){
			sprintf(line, " %p", (void *)f->table[i][j].action);
			strcat(output, line);
		}
		sprintf(line, "\n");	
		strcat(output, line);
	}
	return output;
}

/* dummy apps */
int app1(sm_event *e) {
	printf("\nAPP1 invoked");
	printf("\nprocessed event Id : %lu", e->id);
	printf("\nAPP1 done");
	return 0;
}
	
int app2(sm_event *e) {
	printf("\nAPP2 invoked");
	printf("\nprocessed event Id : %lu", e->id);
	printf("\nAPP2 done");
	return 0;
}

int app3(sm_event *e) {
	printf("\nAPP3 invoked");
	printf("\nprocessed event Id : %lu", e->id);
	printf("\nAPP3 done");
	return 0;
}

int noap(sm_event *e) {
	printf("\nNOAP invoked");
	printf("\ndoing nothing");
	printf("\nNOAP done");
	return 0;
}


/* driver */

int main(int argc,char* argv[])
{   
	if(argc < 2) {
		printf("\nusage: %s json-filename\n", argv[0]);
		exit(0);
	}
	char fn[1024];
	FILE * file;
	file = fopen(argv[1] , "r");
	if (!file) {
		REPORT(ERROR, "Input FSM filename missing or incorrect\n");
		exit(0);
	}
	printf("\nSM.EXEC FSM module test\ninput filename: %s\n", argv[1]);
	char jstr[32 * 1024];
	printf("reading file... \n");
	int i=0;
	while(!feof(file))
    	jstr[i++] = fgetc(file);
	jstr[--i]='\0';
	fclose(file);
	printf("json scanned...\n");
	sm_app_table *at;
	at = sm_app_table_create();
	at = sm_app_table_set(at, "APP1", app1);
	at = sm_app_table_set(at, "APP2", app2);
	at = sm_app_table_set(at, "APP3", app3);
	at = sm_app_table_set(at, "NOAP", noap);
	printf("application regestry created...\n");
	sm_fsm *fsm = sm_fsm_create(jstr, at);
	if(fsm != NULL) {
		printf("fsm created...\n");
		char *res = to_string(fsm);
		printf("%s", res);
		free(res);
		return EXIT_FAILURE;
	}
    return EXIT_SUCCESS;
}