#ifndef CTIME_H
#define CTIME_H

#include "type.h"

#include <stddef.h>

#define CTIME_BUF_SIZE 24

u64 c_time();
int c_time_format_utc(char *buf, size_t size, u64 time);

int c_sleep(u32 ms);
int c_timer(u32 ms);

#endif
