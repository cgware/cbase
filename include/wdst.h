#ifndef WDST_H
#define WDST_H

#include <stdarg.h>
#include <stddef.h>

typedef struct wstrv_s {
	size_t len;
	const wchar_t *data;
} wstrv_t;

// clang-format off
#define WSTRVTN(_str, _len) {.len = _len, .data = _str}
// clang-format on
#define WSTRVT(_str)	   WSTRVTN(_str, sizeof(_str) / sizeof(wchar_t) - 1)
#define WSTRVN(_str, _len) ((wstrv_t)WSTRVTN(_str, _len))
#define WSTRV(_str)	   ((wstrv_t)WSTRVT(_str))
#define WSTRVS(_str)	   WSTRVN((_str).data, (_str).len)
#define WSTRV_NULL	   ((wstrv_t){0})

typedef struct wdst_s wdst_t;
typedef size_t (*wdputs_fn)(wdst_t wdst, wstrv_t str);
typedef size_t (*wdputv_fn)(wdst_t wdst, const wchar_t *fmt, va_list args);
struct wdst_s {
	wdputs_fn wputs;
	wdputv_fn wputv;
	size_t size;
	size_t off;
	void *dst;
	void *priv;
};

size_t wdputs_std(wdst_t dst, wstrv_t str);
size_t wdputv_std(wdst_t dst, const wchar_t *fmt, va_list args);

size_t wdputs_buf(wdst_t dst, wstrv_t str);
size_t wdputv_buf(wdst_t dst, const wchar_t *fmt, va_list args);

size_t wdputs(wdst_t dst, wstrv_t str);
size_t wdputv(wdst_t dst, const wchar_t *fmt, va_list args);
size_t wdputf(wdst_t dst, const wchar_t *fmt, ...);

#define WDST_NONE()	       ((wdst_t){0})
#define WDST_STD()	       ((wdst_t){.wputs = wdputs_std, .wputv = wdputv_std})
#define WDST_BUFN(_buf, _size) ((wdst_t){.wputs = wdputs_buf, .wputv = wdputv_buf, .dst = _buf, .size = _size})
#define WDST_BUF(_buf)	       WDST_BUFN(_buf, sizeof(_buf))

#endif
