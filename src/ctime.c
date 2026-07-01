#include "ctime.h"

#include "platform.h"
#include "print.h"

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
	SYSTEMTIME st;
	GetSystemTime(&st);
	const ctime_t now = {
		.sec  = time(NULL),
		.msec = st.wMilliseconds,
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

const char *c_time_str(char *buf)
{
	if (buf == NULL) {
		return NULL;
	}

	struct tm *timeinfo;
	const ctime_t now = get_time();
#if defined(C_WIN)
	struct tm ti;
	gmtime_s(&ti, &now.sec);
	timeinfo = &ti;
#else
	timeinfo = gmtime(&now.sec);
#endif

	strftime(buf, CTIME_BUF_SIZE, "%Y-%m-%d %H:%M:%S", timeinfo);
	c_sprintf(buf, CTIME_BUF_SIZE, 19, ".%03ld", now.msec);

	return buf;
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
