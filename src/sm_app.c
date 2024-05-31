/* SM.EXEC
   Apps
   (c) anton.bondarenko@gmail.com */

#include <dlfcn.h>
#include <string.h>
#include "sm_app.h"
#include "sm_sys.h"
#include "sm_logger.h"

void *sm_app_loadlib(const char *fn) {
	void *laddr = dlopen(fn, SM_APP_RTLD_FLAG);
	if (!laddr)
		SM_LOG(SM_CORE, SM_LOG_ERR, dlerror());
	return laddr;
}

sm_app sm_app_lookup(void * lib, const char *name) {
	char *error;
	sm_app app = (sm_app)dlsym(lib, name);
	if ((error = dlerror()) != NULL)
		SM_LOG(SM_CORE, SM_LOG_ERR, error);
	return app;
}
