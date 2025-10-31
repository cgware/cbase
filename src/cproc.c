#include "cproc.h"

#include "platform.h"

#include <stdlib.h>
#ifdef C_LINUX
	#include <sys/wait.h>
#endif

int cproc_system(const char *cmd)
{
	int ret = system(cmd);
#ifdef C_LINUX
	if (WIFEXITED(ret)) {
		ret = WEXITSTATUS(ret);
	}
#endif
	return ret;
}
