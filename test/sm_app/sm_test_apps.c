/* SM.EXEC 
   Test applications
   anton.bondarenko@gmail.com */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../../src/sm_sys.h"
#include "../../src/sm_app.h"
#include "../../src/sm_event.h"

int sm_test_app1(sm_event *e) {
	printf("test app # 1 invoked\n");
	printf("event Id: %s\n", (char *)e->data);
	return 0;	
}

int sm_test_app2(sm_event *e) {
	printf("test app # 2 invoked\n");
	printf("event Id: %s\n", (char *)e->data);
	return 0;	
}

int sm_test_app3(sm_event *e) {
	printf("test app # 3 invoked\n");
	printf("event Id: %s\n", (char *)e->data);
	return 0;	
}

int sm_test_app4(sm_event *e) {
	printf("test app # 4 invoked\n");
	printf("event Id: %s\n", (char *)e->data);
	return 0;	
}

int sm_nope(sm_event *e) {
	printf("nope app invoked\n");
	printf("event Id: %s\n", (char *)e->data);
	return 0;	
}