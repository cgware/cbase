#ifndef PRINT_H
#define PRINT_H

#include <stdarg.h>
#include <stddef.h>

int c_printv(const char *fmt, va_list args);
int c_printf(const char *fmt, ...);

int c_sprintv(char *buf, size_t size, int off, const char *fmt, va_list args);
int c_sprintf(char *buf, size_t size, int off, const char *fmt, ...);

typedef struct print_dst_s print_dst_t;
typedef int (*c_printv_fn)(print_dst_t dst, const char *fmt, va_list args);
struct print_dst_s {
	c_printv_fn cb;
	size_t size;
	int off;
	void *dst;
	void *priv;
};

int c_printv_cb(print_dst_t dst, const char *fmt, va_list args);
int c_sprintv_cb(print_dst_t dst, const char *fmt, va_list args);

int c_dprintv(print_dst_t dst, const char *fmt, va_list args);
int c_dprintf(print_dst_t dst, const char *fmt, ...);

#define PRINT_DST_NONE()		 ((print_dst_t){0})
#define PRINT_DST_STD()			 ((print_dst_t){.cb = c_printv_cb})
#define PRINT_DST_BUF(_buf, _size, _off) ((print_dst_t){.cb = c_sprintv_cb, .dst = _buf, .size = _size, .off = _off})

#endif
