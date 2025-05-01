#include "dst.h"

#include "print.h"

#include <stdio.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

size_t dputs_std(dst_t dst, strv_t str)
{
	(void)dst;
	return fwrite(str.data, sizeof(char), str.len, stdout);
}

size_t dputv_std(dst_t dst, const char *fmt, va_list args)
{
	(void)dst;
	int len = c_printv(fmt, args);
	return MAX(len, 0);
}

size_t dputs_buf(dst_t dst, strv_t str)
{
	if (dst.dst == NULL || str.data == NULL || str.len + 1 > dst.size / sizeof(char) - dst.off) {
		return 0;
	}

	for (size_t i = 0; i < str.len; i++) {
		((char *)dst.dst)[dst.off++] = str.data[i];
	}

	((char *)dst.dst)[dst.off++] = '\0';

	return str.len;
}

size_t dputv_buf(dst_t dst, const char *fmt, va_list args)
{
	int len = c_sprintv((char *)dst.dst, dst.size, dst.off, fmt, args);
	return MAX(len, 0);
}

size_t dputs(dst_t dst, strv_t str)
{
	return dst.puts ? dst.puts(dst, str) : 0;
}

size_t dputv(dst_t dst, const char *fmt, va_list args)
{
	return dst.putv ? dst.putv(dst, fmt, args) : 0;
}

size_t dputf(dst_t dst, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	size_t ret = dputv(dst, fmt, args);
	va_end(args);
	return ret;
}
