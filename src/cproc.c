#include "cproc.h"

#include "platform.h"

#include <stdlib.h>
#ifdef C_LINUX
	#include <sys/wait.h>
	#include <unistd.h>
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

int cproc_getpid()
{
#if defined(C_LINUX)
	return getpid();
#else
	return 0;
#endif
}

int cproc_setalarm(alarm_cb cb)
{
#if defined(C_LINUX)
	struct sigaction sa = {0};
	sa.sa_handler	    = cb;
	return sigemptyset(&sa.sa_mask) < 0 || sigaction(SIGALRM, &sa, NULL) < 0;
#else
	(void)cb;
	return 1;
#endif
}
