#include "csock.h"

#include "cerr.h"
#include "platform.h"

#include <errno.h>
#include <stdint.h>
#include <string.h>

#if defined(C_LINUX)
	#include <fcntl.h>
	#include <sys/socket.h>
	#include <sys/un.h>
	#include <unistd.h>
#endif

#if defined(C_LINUX)
static void *csock_from_fd(int fd)
{
	return (void *)((uintptr_t)fd + 1);
}

static int csock_to_fd(void *sock)
{
	return (int)((uintptr_t)sock - 1);
}
#endif

cerr_t csock_open(csock_family_t family, csock_type_t type, int protocol, void **sock)
{
	if (sock == NULL) {
		return CERR_VAL;
	}

	cerr_t ret = CERR_UNKNOWN;

	errno = 0;
#if defined(C_LINUX)
	int f;
	switch (family) {
	case CSOCK_FAMILY_UNIX: f = AF_UNIX; break;
	default: return CERR_VAL;
	}

	int t;
	switch (type) {
	case CSOCK_TYPE_STREAM: t = SOCK_STREAM; break;
	default: return CERR_VAL;
	}

	errno  = 0;
	int fd = socket(f, t, protocol);
	if (fd < 0) {
		int e = errno;
		switch (e) {
		case EINVAL:
		case EPROTONOSUPPORT: ret = CERR_PROTO; break;
		}
	} else {
		*sock = csock_from_fd(fd);
		ret   = CERR_OK;
	}

#else
	ret = CERR_UNSUPPORTED;
#endif
	return ret;
}

cerr_t csock_close(void *sock)
{
	if (sock == NULL) {
		return CERR_VAL;
	}

	cerr_t ret = CERR_UNKNOWN;

#if defined(C_LINUX)
	errno = 0;
	if (close(csock_to_fd(sock)) < 0) {
		int e = errno;
		switch (e) {
		case EBADF: ret = CERR_DESC; break;
		}
	} else {
		ret = CERR_OK;
	}
#else
	return CERR_UNSUPPORTED;
#endif
	return ret;
}

cerr_t csock_setopt(void *sock, csock_opt_t opt, void *val, size_t size)
{
	if (sock == NULL || val == NULL) {
		return CERR_VAL;
	}

	cerr_t ret = CERR_UNKNOWN;

#if defined(C_LINUX)
	int name;
	switch (opt) {
	case CSOCK_OPT_SNDBUF: name = SO_SNDBUF; break;
	default: return CERR_VAL;
	}

	errno = 0;
	if (setsockopt(csock_to_fd(sock), SOL_SOCKET, name, val, size) < 0) {
		int e = errno;
		switch (e) {
		case EBADF: ret = CERR_DESC; break;
		}
	} else {
		ret = CERR_OK;
	}
#else
	ret = CERR_UNSUPPORTED;
#endif

	return ret;
}

cerr_t csock_get_flags(void *sock, int *flags)
{
	if (sock == NULL || flags == NULL) {
		return CERR_VAL;
	}

	cerr_t ret = CERR_UNKNOWN;

#if defined(C_LINUX)
	errno = 0;
	int f = fcntl(csock_to_fd(sock), F_GETFL, 0);
	if (f < 0) {
		int e = errno;
		switch (e) {
		case EBADF: ret = CERR_DESC; break;
		}
	} else {
		*flags = f;
		ret    = CERR_OK;
	}
#else
	ret = CERR_UNSUPPORTED;
#endif

	return ret;
}

cerr_t csock_set_flags(void *sock, int flags)
{
	if (sock == NULL) {
		return CERR_VAL;
	}

	cerr_t ret = CERR_UNKNOWN;

#if defined(C_LINUX)
	errno = 0;
	if (fcntl(csock_to_fd(sock), F_SETFL, flags) < 0) {
		int e = errno;
		switch (e) {
		case EBADF: ret = CERR_DESC; break;
		}
	} else {
		ret = CERR_OK;
	}
#else
	ret = CERR_UNSUPPORTED;
#endif

	return ret;
}

cerr_t csock_bind(void *sock, csock_family_t family, const char *path, size_t len)
{
	if (sock == NULL || path == NULL) {
		return CERR_VAL;
	}

	cerr_t ret = CERR_UNKNOWN;

#if defined(C_LINUX)
	struct sockaddr_un addr = {0};
	if (len > sizeof(addr.sun_path)) {
		return CERR_VAL;
	}

	int f;
	switch (family) {
	case CSOCK_FAMILY_UNIX: f = AF_UNIX; break;
	default: return CERR_VAL;
	}

	addr.sun_family = f;
	memcpy(addr.sun_path, path, len);

	errno = 0;
	if (bind(csock_to_fd(sock), (const struct sockaddr *)&addr, sizeof(addr)) < 0) {
		int e = errno;
		switch (e) {
		case EBADF: ret = CERR_DESC; break;
		case EADDRINUSE: ret = CERR_EXIST; break;
		}
	} else {
		ret = CERR_OK;
	}
#else
	ret = CERR_UNSUPPORTED;
#endif

	return ret;
}

cerr_t csock_listen(void *sock, int n)
{
	if (sock == NULL) {
		return CERR_VAL;
	}

	cerr_t ret = CERR_UNKNOWN;

#if defined(C_LINUX)
	errno = 0;
	if (listen(csock_to_fd(sock), n) < 0) {
		int e = errno;
		switch (e) {
		case EBADF: ret = CERR_DESC; break;
		}
	} else {
		ret = CERR_OK;
	}
#else
	ret = CERR_UNSUPPORTED;
#endif

	return ret;
}

cerr_t csock_connect(void *sock, csock_family_t family, const char *path, size_t len)
{
	if (sock == NULL || path == NULL) {
		return CERR_VAL;
	}

	cerr_t ret = CERR_UNKNOWN;

#if defined(C_LINUX)
	struct sockaddr_un addr = {0};
	if (len > sizeof(addr.sun_path)) {
		return CERR_VAL;
	}

	int f;
	switch (family) {
	case CSOCK_FAMILY_UNIX: f = AF_UNIX; break;
	default: return CERR_VAL;
	}

	addr.sun_family = f;
	memcpy(addr.sun_path, path, len);

	errno = 0;
	if (connect(csock_to_fd(sock), (const struct sockaddr *)&addr, sizeof(addr)) < 0) {
		int e = errno;
		switch (e) {
		case EBADF: ret = CERR_DESC; break;
		case ENOENT: ret = CERR_NOT_FOUND; break;
		case ECONNREFUSED: ret = CERR_CONN; break;
		}
	} else {
		ret = CERR_OK;
	}
#else
	ret = CERR_UNSUPPORTED;
#endif

	return ret;
}

cerr_t csock_accept(void *sock, void **fd)
{
	if (sock == NULL || fd == NULL) {
		return CERR_VAL;
	}

	cerr_t ret = CERR_UNKNOWN;

#if defined(C_LINUX)
	errno = 0;
	int f = accept(csock_to_fd(sock), NULL, NULL);
	if (f < 0) {
		int e = errno;
		switch (e) {
		case EBADF: ret = CERR_DESC; break;
		case EAGAIN: ret = CERR_AGAIN; break;
		case EINVAL: ret = CERR_STATE; break;
		}
	} else {
		*fd = csock_from_fd(f);
		ret = CERR_OK;
	}
#else
	ret = CERR_UNSUPPORTED;
#endif

	return ret;
}

cerr_t csock_write(void *sock, const void *data, size_t size, size_t *n)
{
	if (sock == NULL || data == NULL) {
		return CERR_VAL;
	}

	cerr_t ret = CERR_UNKNOWN;

#if defined(C_LINUX)
	errno	  = 0;
	ssize_t w = write(csock_to_fd(sock), data, size);
	if (w < 0) {
		int e = errno;
		switch (e) {
		case EINTR: ret = CERR_INTERRUPT; break;
		case EBADF: ret = CERR_DESC; break;
		case EAGAIN: ret = CERR_AGAIN; break;
		case ENOTCONN: ret = CERR_CONN; break;
		}
	} else {
		if (n) {
			*n = (size_t)w;
		}

		ret = CERR_OK;
	}

#else
	ret = CERR_UNSUPPORTED;
#endif

	return ret;
}

cerr_t csock_read(void *sock, void *data, size_t size, size_t *n)
{
	if (sock == NULL || data == NULL) {
		return CERR_VAL;
	}

	cerr_t ret = CERR_UNKNOWN;

#if defined(C_LINUX)
	errno	  = 0;
	ssize_t r = read(csock_to_fd(sock), data, size);
	if (r < 0) {
		int e = errno;
		switch (e) {
		case EINTR: ret = CERR_INTERRUPT; break;
		case EBADF: ret = CERR_DESC; break;
		case EAGAIN: ret = CERR_AGAIN; break;
		case EINVAL:
		case ENOTCONN: ret = e == ENOTCONN ? CERR_CONN : CERR_STATE; break;
		}
	} else {
		if (n) {
			*n = (size_t)r;
		}

		ret = CERR_OK;
	}

#else
	ret = CERR_UNSUPPORTED;
#endif

	return ret;
}
