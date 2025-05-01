#ifndef PRINT_H
#define PRINT_H

#include <stdarg.h>
#include <stddef.h>

void c_print_init();

int c_printv(const char *fmt, va_list args);
int c_printf(const char *fmt, ...);

int c_sprintv(char *buf, size_t size, size_t off, const char *fmt, va_list args);
int c_sprintf(char *buf, size_t size, size_t off, const char *fmt, ...);

#endif
