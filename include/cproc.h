#ifndef CPROC_H
#define CPROC_H

int cproc_system(const char *cmd);
int cproc_getpid();

typedef void (*alarm_cb)(int sig);
int cproc_setalarm(alarm_cb cb);

#endif
