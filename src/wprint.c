#include "wprint.h"

#include "platform.h"

#include <stdio.h>
#include <wchar.h>

#if defined(C_WIN)
	#include <io.h>
#else
	#include <locale.h>
#endif

void c_startw(void *file)
{
	fflush(file);
	setlocale(LC_ALL, "");
	file = freopen(NULL, "w", file);
}

void c_endw(void *file)
{
	fflush(file);
	file = freopen(NULL, "w", file);
}

int c_wprintv(const wchar_t *fmt, va_list args)
{
	if (fmt == NULL) {
		return 0;
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

int c_swprintv(wchar_t *buf, size_t size, int off, const wchar_t *fmt, va_list args)
{
	if ((buf == NULL && size > 0) || off * sizeof(wchar_t) > size || fmt == NULL) {
		return 0;
	}

	buf = buf == NULL ? buf : &buf[off];

	va_list copy;
	va_copy(copy, args);
	int ret;
#if defined(C_WIN)
	#pragma warning(push)
	#pragma warning(disable : 6387)
	ret = vswprintf_s(buf, size / sizeof(wchar_t) - off, fmt, copy);
	#pragma warning(push)
	va_end(copy);
#else
	ret = vswprintf(buf, size / sizeof(wchar_t) - off, fmt, copy);
	va_end(copy);

	if (size > 0 && (size_t)ret > size - off) {
		return 0;
	}
#endif
	return ret;
}

int c_swprintf(wchar_t *buf, size_t size, int off, const wchar_t *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int ret = c_swprintv(buf, size, off, fmt, args);
	va_end(args);
	return ret;
}

int c_wprintv_cb(wprint_dst_t dst, const wchar_t *fmt, va_list args)
{
	(void)dst;
	return c_wprintv(fmt, args);
}

int c_swprintv_cb(wprint_dst_t dst, const wchar_t *fmt, va_list args)
{
	return c_swprintv((wchar_t *)dst.dst, dst.size, dst.off, fmt, args);
}

int c_dwprintv(wprint_dst_t dst, const wchar_t *fmt, va_list args)
{
	if (dst.cb == NULL) {
		return 0;
	}

	return dst.cb(dst, fmt, args);
}

int c_dwprintf(wprint_dst_t dst, const wchar_t *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int ret = c_dwprintv(dst, fmt, args);
	va_end(args);
	return ret;
}
