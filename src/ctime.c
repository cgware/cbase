#if !defined(_WIN32)
	#define _POSIX_C_SOURCE 200809L
#endif

#include "ctime.h"

#include "platform.h"

#include <stdio.h>
#include <time.h>

#if defined(C_WIN)
	#include <windows.h>
#else
	#include <sys/time.h>
	#include <unistd.h>
#endif

typedef struct ctime_s {
	time_t sec;
	u32 msec;
} ctime_t;

static ctime_t get_time()
{
#if defined(C_WIN)
	FILETIME file_time;
	GetSystemTimeAsFileTime(&file_time);
	ULARGE_INTEGER ticks = {
		.LowPart  = file_time.dwLowDateTime,
		.HighPart = file_time.dwHighDateTime,
	};
	u64 milliseconds  = (ticks.QuadPart - 116444736000000000ULL) / 10000;
	const ctime_t now = {
		.sec  = (time_t)(milliseconds / 1000),
		.msec = (u32)(milliseconds % 1000),
	};
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	const ctime_t now = {
		.sec  = tv.tv_sec,
		.msec = tv.tv_usec / 1000,
	};
#endif
	return now;
}

u64 c_time()
{
	const ctime_t now = get_time();
	return (u64)now.sec * 1000 + (u64)now.msec;
}

int c_time_format_utc(char *buf, size_t size, u64 time)
{
	if (buf == NULL || size < CTIME_BUF_SIZE) {
		return 1;
	}

	time_t seconds = (time_t)(time / 1000);
	struct tm utc;
#if defined(C_WIN)
	if (gmtime_s(&utc, &seconds) != 0) {
		return 1;
	}
#else
	if (gmtime_r(&seconds, &utc) == NULL) {
		return 1; // LCOV_EXCL_LINE: u64 milliseconds cannot exceed gmtime_r's calendar range on Linux.
	}
#endif

	if (strftime(buf, size, "%Y-%m-%d %H:%M:%S", &utc) != 19) {
		return 1;
	}
	return snprintf(buf + 19, size - 19, ".%03u", (u32)(time % 1000)) == 4 ? 0 : 1;
}

int c_sleep(u32 ms)
{
#if defined(C_WIN)
	Sleep((DWORD)ms);
	return 0;
#else
	struct timeval tv;
	tv.tv_sec  = ms / 1000;
	tv.tv_usec = ms % 1000 * 1000;
	select(0, NULL, NULL, NULL, &tv);
	return 0;
#endif
}

int c_timer(u32 ms)
{
#if defined(C_LINUX)
	struct itimerval tv = {0};
	tv.it_value.tv_sec  = (long)(ms / 1000);
	tv.it_value.tv_usec = (long)((ms % 1000) * 1000);
	return setitimer(ITIMER_REAL, &tv, NULL);
#else
	return 1;
#endif
}
