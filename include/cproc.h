#ifndef CPROC_H
#define CPROC_H

#include <stddef.h>

int cproc_system(const char *cmd);
int cproc_getpid();
const char *cproc_getenv(const char *name);
int cproc_gethostname(char *name, size_t len);

typedef void (*alarm_cb)(int sig);
int cproc_setalarm(alarm_cb cb);

#endif
