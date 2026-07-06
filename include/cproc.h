#ifndef CPROC_H
#define CPROC_H

#include <stddef.h>

int cproc_system(const char *cmd);

int cproc_getpid();
int cproc_gethostname(char *name, size_t len);

const char *cproc_getenv(const char *name);
int cproc_setenv(const char *name, const char *val, int overwrite);
int cproc_unsetenv(const char *name);

int cproc_dlopen(const char *name, void **lib);
int cproc_dlsym(void *lib, const char *name, void **sym);
int cproc_dlclose(void *lib);

typedef void (*alarm_cb)(int sig);
int cproc_setalarm(alarm_cb cb);

#endif
