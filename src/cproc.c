#include "cproc.h"

#include "platform.h"

#include <stdlib.h>

#if defined(C_WIN)
	#include <windows.h>
#else
	#include <dlfcn.h>
	#include <sys/wait.h>
	#include <unistd.h>
#endif

#if defined(C_WIN)
static char cproc_env[32767];
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
#elif defined(C_WIN)
	return (int)GetCurrentProcessId();
#else
	return 0;
#endif
}

int cproc_gethostname(char *name, size_t len)
{
#if defined(C_LINUX)
	return gethostname(name, len);
#elif defined(C_WIN)
	DWORD size = (DWORD)len;
	return !GetComputerNameA(name, &size);
#else
	(void)name;
	(void)len;
	return 1;
#endif
}

const char *cproc_getenv(const char *name)
{
#if defined(C_LINUX)
	return getenv(name);
#elif defined(C_WIN)
	DWORD len;

	SetLastError(ERROR_SUCCESS);
	len = GetEnvironmentVariableA(name, cproc_env, sizeof(cproc_env));
	if ((len == 0 && GetLastError() != ERROR_SUCCESS) || len >= sizeof(cproc_env)) {
		return NULL;
	}
	return cproc_env;
#else
	return NULL;
#endif
}

int cproc_setenv(const char *name, const char *val, int overwrite)
{
#if defined(C_LINUX)
	return setenv(name, val, overwrite);
#elif defined(C_WIN)
	if (!overwrite && cproc_getenv(name) != NULL) {
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

int cproc_dlopen(const char *name, void **lib)
{
	if (lib == NULL) {
		return 1;
	}

#if defined(C_WIN)
	*lib = LoadLibraryA(name);
#elif defined(C_LINUX)
	*lib = dlopen(name, RTLD_LAZY);
#else
	(void)name;
	*lib = NULL;
#endif

	return *lib == NULL;
}

int cproc_dlsym(void *lib, const char *name, void **sym)
{
	if (lib == NULL || sym == NULL) {
		return 1;
	}

#if defined(C_WIN)
	*sym = GetProcAddress(lib, name);
#elif defined(C_LINUX)
	dlerror();
	*sym = dlsym(lib, name);
	return dlerror() != NULL;
#else
	(void)name;
	*sym = NULL;
#endif

	return *sym == NULL;
}

int cproc_dlclose(void *lib)
{
	if (lib == NULL) {
		return 1;
	}

#if defined(C_WIN)
	return !FreeLibrary(lib);
#elif defined(C_LINUX)
	return dlclose(lib);
#else
	return 1;
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
