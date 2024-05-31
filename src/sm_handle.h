/* SM.EXEC
   External handle plugin class
   (c) anton.bondarenko@gmail.com */

#ifndef SM_HANDLE_H
#define SM_HANDLE_H

typedef struct sm_handle {
	void (* free)(void *);
	void * object;
} sm_handle;

#endif //SM_HANDLE_H