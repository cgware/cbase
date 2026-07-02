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

const char *cproc_getenv(const char *name)
{
#if defined(C_LINUX)
	return getenv(name);
#elif defined(C_WIN)
	return getenv(name);
#else
	return NULL;
#endif
}

int cproc_setenv(const char *name, const char *val, int overwrite)
{
#if defined(C_LINUX)
	return setenv(name, val, overwrite);
#elif defined(C_WIN)
	if (!overwrite && getenv(name) != NULL) {
		return 0;
	}
	return _putenv_s(name, val);
#else
	(void)name;
	(void)val;
	(void)overwrite;
	return 1;
#endif
}

int cproc_unsetenv(const char *name)
{
#if defined(C_LINUX)
	return unsetenv(name);
#elif defined(C_WIN)
	return _putenv_s(name, "");
#else
	(void)name;
	return 1;
#endif
}

int cproc_gethostname(char *name, size_t len)
{
#if defined(C_LINUX)
	return gethostname(name, len);
#else
	return NULL;
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
