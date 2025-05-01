#include "print.h"

#include "platform.h"

#include <locale.h>
#include <stdio.h>

void c_print_init()
{
	setlocale(LC_ALL, "en_US.UTF-8");
}

int c_printv(const char *fmt, va_list args)
{
	if (fmt == NULL) {
		return -1;
	}

	va_list copy;
	va_copy(copy, args);
	int ret = vprintf(fmt, copy);
	va_end(copy);
	return ret;
}

int c_printf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int ret = c_printv(fmt, args);
	va_end(args);
	return ret;
}

int c_sprintv(char *buf, size_t size, size_t off, const char *fmt, va_list args)
{
	if ((buf == NULL && size > 0) || off * sizeof(char) > size || fmt == NULL) {
		return -1;
	}

	buf = buf == NULL ? buf : &buf[off];

	va_list copy;
	va_copy(copy, args);
	int ret;
#if defined(C_WIN)
	if (buf == NULL && size == 0) {
		ret = vsnprintf(buf, size / sizeof(char) - off, fmt, copy);
	} else {
		ret = vsnprintf_s(buf, size / sizeof(char) - off, size / sizeof(char) - off, fmt, copy);
	}
	va_end(copy);
#else
	ret = vsnprintf(buf, size / sizeof(char) - off, fmt, copy);
	va_end(copy);
	if (size > 0 && (size_t)ret > size / sizeof(char) - off) {
		return -1;
	}
#endif
	return ret;
}

int c_sprintf(char *buf, size_t size, size_t off, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int ret = c_sprintv(buf, size, off, fmt, args);
	va_end(args);
	return ret;
}
