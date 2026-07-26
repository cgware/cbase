#include "print.h"

#include "platform.h"

#include <locale.h>
#include <stdio.h>
#if defined(C_WIN) && defined(_MSC_VER)
	#include <stdint.h>
	#include <stdlib.h>
	#include <wchar.h>
#endif

#if defined(C_WIN) && defined(_MSC_VER)
static void c_print_invalid_parameter(const wchar_t *expression, const wchar_t *function, const wchar_t *file, unsigned int line,
				      uintptr_t reserved)
{
	(void)expression;
	(void)function;
	(void)file;
	(void)line;
	(void)reserved;
}

static int c_vsnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
	_invalid_parameter_handler old = _set_thread_local_invalid_parameter_handler(c_print_invalid_parameter);
	int ret			       = vsnprintf(buf, size, fmt, args);
	_set_thread_local_invalid_parameter_handler(old);
	return ret;
}
#else
static int c_vsnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
	return vsnprintf(buf, size, fmt, args);
}
#endif

static int c_printf_digit(char c)
{
	return c >= '0' && c <= '9';
}

static int c_printf_spec(char c)
{
	switch (c) {
	case 'd':
	case 'i':
	case 'o':
	case 'u':
	case 'x':
	case 'X':
	case 'f':
	case 'F':
	case 'e':
	case 'E':
	case 'g':
	case 'G':
	case 'a':
	case 'A':
	case 'c':
	case 'C':
	case 's':
	case 'S':
	case 'p':
	case 'Z': return 1;
	default: return 0;
	}
}

static int c_printf_fmt_valid(const char *fmt)
{
	while (*fmt) {
		if (*fmt++ != '%') {
			continue;
		}
		if (*fmt == '%') {
			fmt++;
			continue;
		}
		while (*fmt == '-' || *fmt == '+' || *fmt == ' ' || *fmt == '#' || *fmt == '0') {
			fmt++;
		}
		if (*fmt == '*') {
			fmt++;
		} else {
			while (c_printf_digit(*fmt)) {
				fmt++;
			}
		}
		if (*fmt == '.') {
			fmt++;
			if (*fmt == '*') {
				fmt++;
			} else {
				while (c_printf_digit(*fmt)) {
					fmt++;
				}
			}
		}
		if (*fmt == 'h' && fmt[1] == 'h') {
			fmt += 2;
		} else if (*fmt == 'l' && fmt[1] == 'l') {
			fmt += 2;
		} else if (*fmt == 'I' && c_printf_digit(fmt[1]) && c_printf_digit(fmt[2])) {
			fmt += 3;
		} else if (*fmt == 'h' || *fmt == 'l' || *fmt == 'j' || *fmt == 'z' || *fmt == 't' || *fmt == 'L' || *fmt == 'I') {
			fmt++;
		}
		if (*fmt == '\0' || !c_printf_spec(*fmt)) {
			return 0;
		}
		fmt++;
	}
	return 1;
}

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
	if (!c_printf_fmt_valid(fmt)) {
		return -1;
	}

	size_t available = size / sizeof(char) - off;

	va_list copy;
	va_copy(copy, args);
	int ret = c_vsnprintf(NULL, 0, fmt, copy);
	va_end(copy);
	if (ret < 0) {
		return ret;
	}
	if (buf == NULL) {
		return ret;
	}
	if ((size_t)ret >= available) {
		return -1;
	}

	buf = &buf[off];

	va_copy(copy, args);
	int write = c_vsnprintf(buf, available, fmt, copy);
	va_end(copy);
	if (write != ret) {
		return -1; // LCOV_EXCL_LINE
	}
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
