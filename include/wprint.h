#ifndef WPRINT_H
#define WPRINT_H

#include <stdarg.h>
#include <stddef.h>

void c_startw(void *file);
void c_endw(void *file);

int c_wprintv(const wchar_t *fmt, va_list args);
int c_wprintf(const wchar_t *fmt, ...);

int c_swprintv(wchar_t *buf, size_t size, int off, const wchar_t *fmt, va_list args);
int c_swprintf(wchar_t *buf, size_t size, int off, const wchar_t *fmt, ...);

typedef struct wprint_dst_s wprint_dst_t;
typedef int (*c_wprintv_fn)(wprint_dst_t dst, const wchar_t *fmt, va_list args);
struct wprint_dst_s {
	c_wprintv_fn cb;
	size_t size;
	int off;
	void *dst;
	void *priv;
};

int c_wprintv_cb(wprint_dst_t dst, const wchar_t *fmt, va_list args);
int c_swprintv_cb(wprint_dst_t dst, const wchar_t *fmt, va_list args);

int c_dwprintv(wprint_dst_t dst, const wchar_t *fmt, va_list args);
int c_dwprintf(wprint_dst_t dst, const wchar_t *fmt, ...);

#define PRINT_DST_WNONE()		   ((wprint_dst_t){0})
#define PRINT_DST_WSTD()		   ((wprint_dst_t){.cb = c_wprintv_cb})
#define PRINT_DST_WBUF(_wbuf, _size, _off) ((wprint_dst_t){.cb = c_swprintv_cb, .dst = _wbuf, .size = _size, .off = _off})

#endif
