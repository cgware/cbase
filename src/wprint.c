#include "wprint.h"

#include "platform.h"

#include <locale.h>
#include <stdio.h>
#include <wchar.h>

void c_startw(void *file)
{
	FILE *f = (FILE *)file;
	fflush(f);
#if defined(C_WIN)
	freopen_s(&f, NULL, "w", f);
#else
	f = freopen(NULL, "w", f);
#endif
}

void c_endw(void *file)
{
	FILE *f = (FILE *)file;
	fflush(f);
#if defined(C_WIN)
	freopen_s(&f, NULL, "w", f);
#else
	f = freopen(NULL, "w", f);
#endif
}

int c_wprintv(const wchar_t *fmt, va_list args)
{
	if (fmt == NULL) {
		return -1;
	}

	va_list copy;
	va_copy(copy, args);
	int ret = vwprintf(fmt, copy);
	va_end(copy);
	return ret;
}

int c_wprintf(const wchar_t *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int ret = c_wprintv(fmt, args);
	va_end(args);
	return ret;
}

int c_swprintv(wchar_t *buf, size_t size, size_t off, const wchar_t *fmt, va_list args)
{
	if ((buf == NULL && size > 0) || off * sizeof(wchar_t) > size || fmt == NULL) {
		return -1;
	}

	buf = buf == NULL ? buf : &buf[off];

	va_list copy;
	va_copy(copy, args);
	int ret;
#if defined(C_WIN)
	#pragma warning(disable : 6387)
	ret = vswprintf_s(buf, size / sizeof(wchar_t) - off, fmt, copy);
	#pragma warning(default : 6387)
	va_end(copy);
#else
	ret = vswprintf(buf, size / sizeof(wchar_t) - off, fmt, copy);
	va_end(copy);
	if (size > 0 && (size_t)ret > size / sizeof(wchar_t) - off) {
		return -1;
	}
#endif
	return ret;
}

int c_swprintf(wchar_t *buf, size_t size, size_t off, const wchar_t *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int ret = c_swprintv(buf, size, off, fmt, args);
	va_end(args);
	return ret;
}
