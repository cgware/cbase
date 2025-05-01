#ifndef DST_H
#define DST_H

#include <stdarg.h>
#include <stddef.h>

typedef struct strv_s {
	size_t len;
	const char *data;
} strv_t;

// clang-format off
#define STRVTN(_str, _len) {.len = _len, .data = _str}
// clang-format on
#define STRVT(_str)	  STRVTN(_str, sizeof(_str) - 1)
#define STRVN(_str, _len) ((strv_t)STRVTN(_str, _len))
#define STRV(_str)	  ((strv_t)STRVT(_str))
#define STRVS(_str)	  STRVN((_str).data, (_str).len)
#define STRV_NULL	  ((strv_t){0})

typedef struct dst_s dst_t;
typedef size_t (*dputs_fn)(dst_t dst, strv_t str);
typedef size_t (*dputv_fn)(dst_t dst, const char *fmt, va_list args);
struct dst_s {
	dputs_fn puts;
	dputv_fn putv;
	size_t size;
	size_t off;
	void *dst;
	void *priv;
};

size_t dputs_std(dst_t dst, strv_t str);
size_t dputv_std(dst_t dst, const char *fmt, va_list args);

size_t dputs_buf(dst_t dst, strv_t str);
size_t dputv_buf(dst_t dst, const char *fmt, va_list args);

size_t dputs(dst_t dst, strv_t str);
size_t dputv(dst_t dst, const char *fmt, va_list args);
size_t dputf(dst_t dst, const char *fmt, ...);

#define DST_NONE()	      ((dst_t){0})
#define DST_STD()	      ((dst_t){.puts = dputs_std, .putv = dputv_std})
#define DST_BUFN(_buf, _size) ((dst_t){.puts = dputs_buf, .putv = dputv_buf, .dst = _buf, .size = _size})
#define DST_BUF(_buf)	      DST_BUFN(_buf, sizeof(_buf))

#endif
