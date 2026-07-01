#ifndef CSOCK_H
#define CSOCK_H

#include "cerr.h"

#include <stddef.h>

typedef enum csock_family_e {
	CSOCK_FAMILY_UNKNOWN,
	CSOCK_FAMILY_UNIX,
} csock_family_t;

typedef enum csock_type_e {
	CSOCK_TYPE_UNKNOWN,
	CSOCK_TYPE_STREAM,
} csock_type_t;

typedef enum csock_opt_e {
	CSOCK_OPT_UNKNOWN,
	CSOCK_OPT_SNDBUF,
} csock_opt_t;

cerr_t csock_open(csock_family_t family, csock_type_t type, int protocol, void **sock);
cerr_t csock_close(void *sock);

cerr_t csock_setopt(void *sock, csock_opt_t opt, void *val, size_t size);

cerr_t csock_get_flags(void *sock, int *flags);
cerr_t csock_set_flags(void *sock, int flags);

cerr_t csock_bind(void *sock, csock_family_t family, const char *path, size_t len);
cerr_t csock_listen(void *sock, int n);
cerr_t csock_connect(void *sock, csock_family_t family, const char *path, size_t len);
cerr_t csock_accept(void *sock, void **fd);

cerr_t csock_write(void *sock, const void *data, size_t size, size_t *n);
cerr_t csock_read(void *sock, void *data, size_t size, size_t *n);

#endif
