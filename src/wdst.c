#include "wdst.h"

#include "wprint.h"

#include <stdio.h>
#include <wchar.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

size_t wdputs_std(wdst_t dst, wstrv_t str)
{
	(void)dst;
	int len = c_wprintf(L"%.*ls", (int)str.len, str.data);
	return MAX(len, 0);
}

size_t wdputv_std(wdst_t dst, const wchar_t *fmt, va_list args)
{
	(void)dst;
	int len = c_wprintv(fmt, args);
	return MAX(len, 0);
}

size_t wdputs_buf(wdst_t dst, wstrv_t str)
{
	if (dst.dst == NULL || str.data == NULL || str.len + 1 > dst.size / sizeof(wchar_t) - dst.off) {
		return 0;
	}

	for (size_t i = 0; i < str.len; i++) {
		((wchar_t *)dst.dst)[dst.off++] = str.data[i];
	}

	((wchar_t *)dst.dst)[dst.off++] = L'\0';

	return str.len;
}

size_t wdputv_buf(wdst_t dst, const wchar_t *fmt, va_list args)
{
	int len = c_swprintv((wchar_t *)dst.dst, dst.size, dst.off, fmt, args);
	return MAX(len, 0);
}

size_t wdputs(wdst_t dst, wstrv_t str)
{
	return dst.wputs ? dst.wputs(dst, str) : 0;
}

size_t wdputv(wdst_t dst, const wchar_t *fmt, va_list args)
{
	return dst.wputv ? dst.wputv(dst, fmt, args) : 0;
}

size_t wdputf(wdst_t dst, const wchar_t *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	size_t ret = wdputv(dst, fmt, args);
	va_end(args);
	return ret;
}
