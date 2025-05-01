#ifndef WPRINT_H
#define WPRINT_H

#include <stdarg.h>
#include <stddef.h>

void c_startw(void *file);
void c_endw(void *file);

int c_wprintv(const wchar_t *fmt, va_list args);
int c_wprintf(const wchar_t *fmt, ...);

int c_swprintv(wchar_t *buf, size_t size, size_t off, const wchar_t *fmt, va_list args);
int c_swprintf(wchar_t *buf, size_t size, size_t off, const wchar_t *fmt, ...);

#endif
