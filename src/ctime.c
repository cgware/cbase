#if !defined(_WIN32)
	#define _POSIX_C_SOURCE 200809L
#endif

#include "ctime.h"

#include "platform.h"

#include <stdio.h>

#if defined(C_WIN)
	#include <windows.h>
#else
	#include <sys/time.h>
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
	if (buf == NULL || size < CTIME_BUF_SIZE || time > 253402300799999ULL) {
		return 1;
	}

	u64 seconds	= time / 1000;
	u64 days	= seconds / 86400;
	u64 day_seconds = seconds % 86400;

	u64 z		= days + 719468;
	u64 era		= z / 146097;
	u64 day_of_era	= z - era * 146097;
	u64 year_of_era = (day_of_era - day_of_era / 1460 + day_of_era / 36524 - day_of_era / 146096) / 365;
	u64 year	= year_of_era + era * 400;
	u64 day_of_year = day_of_era - (365 * year_of_era + year_of_era / 4 - year_of_era / 100);
	u64 month_part	= (5 * day_of_year + 2) / 153;
	u64 day		= day_of_year - (153 * month_part + 2) / 5 + 1;
	u64 month	= month_part < 10 ? month_part + 3 : month_part - 9;
	year += month <= 2;

	return snprintf(buf,
			size,
			"%04u-%02u-%02u %02u:%02u:%02u.%03u",
			(u32)year,
			(u32)month,
			(u32)day,
			(u32)(day_seconds / 3600),
			(u32)(day_seconds / 60 % 60),
			(u32)(day_seconds % 60),
			(u32)(time % 1000)) == CTIME_BUF_SIZE - 1
		       ? 0
		       : 1;
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
