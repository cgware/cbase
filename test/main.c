#include "cerr.h"
#include "cfs.h"
#include "cproc.h"
#include "csock.h"
#include "ctime.h"
#include "dst.h"
#include "mem_stats.h"
#include "platform.h"
#include "print.h"
#include "type.h"
#include "wdst.h"
#include "wprint.h"

#include <inttypes.h>
#include <stdio.h>

#define EXPECT(_act, _exp)                                                                                                                 \
	do {                                                                                                                               \
		uintptr_t _actual = (uintptr_t)(_act);                                                                                     \
		uintptr_t _expect = (uintptr_t)(_exp);                                                                                     \
		if (_actual != _expect) {                                                                                                  \
			printf("\033[31m%s:%d: %" PRIXPTR " != %" PRIXPTR "\033[0m\n", __FILE__, __LINE__, _actual, _expect);            \
			ret = 1;                                                                                                           \
		}                                                                                                                          \
	} while (0)

#define TEST_DIR  "t_cfs_tmp"
#define TEST_FILE "t_cfs_file.txt"

static size_t cstrlen(const char *cstr)
{
	size_t len = 0;
	while (*cstr++) {
		len++;
	}
	return len;
}

static int cstreq(const char *a, const char *b)
{
	if (!a || !b) {
		return a == b;
	}
	while (*a && *a == *b) {
		a++;
		b++;
	}
	return *a == *b;
}

static void t_alarm(int sig)
{
	(void)sig;
}

static int t_set_alarm(u32 ms)
{
	if (cproc_setalarm(t_alarm)) {
		return 1;
	}
	return c_timer(ms);
}

static int t_fill_socket(void *sock)
{
	int flags;
	if (csock_get_flags(sock, &flags) != CERR_OK || csock_set_flags(sock, flags | 04000) != CERR_OK) {
		return 1;
	}

	u8 buf[4096] = {0};
	for (;;) {
		cerr_t err = csock_write(sock, buf, sizeof(buf), NULL);
		if (err == CERR_OK) {
			continue;
		}
		if (err == CERR_AGAIN) {
			break;
		}
		if (err != CERR_INTERRUPT) {
			csock_set_flags(sock, flags);
			return 1;
		}
	}

	if (csock_set_flags(sock, flags) != CERR_OK) {
		return 1;
	}

	return 0;
}

static int t_mem_stats()
{
	int ret = 0;

	const mem_stats_t *stats;
	mem_stats_t tmp = {0};

	stats = mem_stats_get();

	mem_stats_set(NULL);
	mem_stats_alloc(1);
	EXPECT(tmp.mem, 0);
	mem_stats_realloc(1, 2);
	EXPECT(tmp.mem, 0);
	mem_stats_free(2);
	EXPECT(tmp.mem, 0);

	EXPECT(mem_stats_set(&tmp) == stats, 1);
	mem_stats_alloc(1);
	EXPECT(tmp.mem, 1);
	mem_stats_realloc(1, 2);
	EXPECT(tmp.mem, 2);
	mem_stats_free(2);
	EXPECT(tmp.mem, 0);

	mem_stats_set((mem_stats_t *)stats);

	return ret;
}

static int t_cerr()
{
	int ret = 0;

	cerr_str(CERR_UNKNOWN + 1);

	return ret;
}

static int t_cproc()
{
	int ret = 0;
	char hostname[256] = {0};
	const char *env = "CBASE_TEST_ENV_8B1E0D7243D44783";
	const char *system_ok;
	const char *system_fail;
	const char *lib_name;
	const char *sym_name;
	int alarm_ret;
	void *lib = NULL;
	void *main = NULL;
	void *sym = NULL;

#ifdef C_WIN
	system_ok   = "cmd /c exit 0";
	system_fail = "cmd /c exit 1";
	lib_name    = "kernel32.dll";
	sym_name    = "GetCurrentProcessId";
	alarm_ret   = 1;
#else
	system_ok   = "true";
	system_fail = "false";
	lib_name    = "libc.so.6";
	sym_name    = "getpid";
	alarm_ret   = 0;
#endif

	EXPECT(cproc_system(system_ok), 0);
	EXPECT(cproc_system(system_fail), 1);
	EXPECT(cproc_getpid() > 0, 1);
	EXPECT(cproc_gethostname(hostname, sizeof(hostname)), 0);
	EXPECT(cstrlen(hostname) > 0, 1);
	EXPECT(cproc_setalarm(t_alarm), alarm_ret);
	EXPECT(cproc_getenv("CBASE_TEST_ENV_DOES_NOT_EXIST_8B1E0D7243D44783"), NULL);
	EXPECT(cproc_dlopen(lib_name, NULL), 1);
	EXPECT(cproc_dlmain(NULL), 1);
	EXPECT(cproc_dlmain(&main), 0);
	EXPECT(main != NULL, 1);
	EXPECT(cproc_dlopen(lib_name, &lib), 0);
	EXPECT(lib != NULL, 1);
	EXPECT(cproc_dlsym(NULL, sym_name, &sym), 1);
	EXPECT(cproc_dlsym(lib, sym_name, NULL), 1);
	EXPECT(cproc_dlsym(lib, "CBASE_TEST_SYMBOL_DOES_NOT_EXIST_8B1E0D7243D44783", &sym), 1);
	EXPECT(sym, NULL);
	EXPECT(cproc_dlsym(lib, sym_name, &sym), 0);
	EXPECT(sym != NULL, 1);
	EXPECT(cproc_dlclose(NULL), 1);
	EXPECT(cproc_dlclose(lib), 0);

	EXPECT(cproc_unsetenv(env), 0);
	EXPECT(cproc_getenv(env), NULL);
	EXPECT(cproc_setenv(env, "one", 0), 0);
	EXPECT(cstreq(cproc_getenv(env), "one"), 1);
	EXPECT(cproc_setenv(env, "two", 0), 0);
	EXPECT(cstreq(cproc_getenv(env), "one"), 1);
	EXPECT(cproc_setenv(env, "two", 1), 0);
	EXPECT(cstreq(cproc_getenv(env), "two"), 1);
	EXPECT(cproc_unsetenv(env), 0);
	EXPECT(cproc_getenv(env), NULL);

	return ret;
}

static int t_cfs()
{
	int ret = 0;

	void *file;
	size_t size;
	char buf[256] = {0};

	EXPECT(cfs_rmdir(NULL), CERR_VAL);
	EXPECT(cfs_rmdir(""), CERR_NOT_FOUND);
	EXPECT(cfs_rmdir(TEST_DIR), CERR_NOT_FOUND);
	EXPECT(cfs_rmdir(TEST_DIR "/" TEST_DIR), CERR_NOT_FOUND);

	EXPECT(cfs_rmfile(NULL), CERR_VAL);
	EXPECT(cfs_rmfile(""), CERR_NOT_FOUND);
	EXPECT(cfs_rmfile(TEST_FILE), CERR_NOT_FOUND);
	EXPECT(cfs_rmfile(TEST_DIR "/" TEST_FILE), CERR_NOT_FOUND);

	EXPECT(cfs_open(NULL, "r", &file), CERR_VAL);
	EXPECT(cfs_open("", "r", &file), CERR_NOT_FOUND);
	EXPECT(cfs_open(TEST_DIR "/" TEST_FILE, NULL, &file), CERR_VAL);
	EXPECT(cfs_open(TEST_DIR "/" TEST_FILE, "f", &file), CERR_VAL);
	EXPECT(cfs_open(TEST_DIR "/" TEST_FILE, "r", NULL), CERR_VAL);
	EXPECT(cfs_open(TEST_FILE, "r", &file), CERR_NOT_FOUND);
	EXPECT(cfs_open(TEST_DIR "/" TEST_FILE, "r", &file), CERR_NOT_FOUND);

	EXPECT(cfs_isdir(NULL), 0);
	EXPECT(cfs_isdir(""), 0);
	EXPECT(cfs_isdir(TEST_DIR "/" TEST_DIR), 0);
	EXPECT(cfs_isdir(TEST_DIR), 0);

	EXPECT(cfs_mkfile(NULL), CERR_VAL);
	EXPECT(cfs_mkfile(""), CERR_NOT_FOUND);
	EXPECT(cfs_mkfile(TEST_DIR "/" TEST_FILE), CERR_NOT_FOUND);

	EXPECT(cfs_mkdir(NULL), CERR_VAL);
	EXPECT(cfs_mkdir(""), CERR_NOT_FOUND);
	EXPECT(cfs_mkdir(TEST_DIR "/" TEST_DIR), CERR_NOT_FOUND);
	EXPECT(cfs_mkdir(TEST_DIR), CERR_OK);
	EXPECT(cfs_mkdir(TEST_DIR), CERR_EXIST);
	EXPECT(cfs_isdir(TEST_DIR), 1);

	EXPECT(cfs_open(TEST_DIR, "r", &file), CERR_TYPE);
	EXPECT(cfs_open(TEST_DIR, "w", &file), CERR_TYPE);

	EXPECT(cfs_isfile(NULL), 0);
	EXPECT(cfs_isfile(""), 0);
	EXPECT(cfs_isfile(TEST_DIR), 0);
	EXPECT(cfs_isfile(TEST_DIR "/" TEST_FILE), 0);
	EXPECT(cfs_mkfile(TEST_DIR), CERR_TYPE);
	EXPECT(cfs_mkfile(TEST_DIR "/" TEST_FILE), CERR_OK);
	EXPECT(cfs_mkfile(TEST_DIR "/" TEST_FILE), CERR_EXIST);
	EXPECT(cfs_isfile(TEST_DIR "/" TEST_FILE), 1);

	EXPECT(cfs_mkdir(TEST_DIR "/" TEST_FILE "/" TEST_DIR), CERR_NOT_FOUND);
	EXPECT(cfs_mkfile(TEST_DIR "/" TEST_FILE "/" TEST_FILE), CERR_NOT_FOUND);

	EXPECT(cfs_open(TEST_DIR "/" TEST_FILE "/" TEST_DIR, "w", &file), CERR_NOT_FOUND);
	EXPECT(cfs_open(TEST_DIR "/" TEST_FILE, "w", &file), CERR_OK);
	EXPECT(cfs_read(file, buf, 1), CERR_DESC);
	EXPECT(cfs_write(NULL, "a", 1), CERR_VAL);
	EXPECT(cfs_write(file, NULL, 1), CERR_VAL);
	EXPECT(cfs_write(file, "a", 1), CERR_OK);

	EXPECT(cfs_close(NULL), CERR_VAL);
	EXPECT(cfs_close(file), CERR_OK);

	EXPECT(cfs_open(TEST_DIR "/" TEST_FILE, "r", &file), CERR_OK);

	EXPECT(cfs_du(NULL, &size), CERR_VAL);
	EXPECT(cfs_du(file, NULL), CERR_VAL);
	EXPECT(cfs_du(file, &size), CERR_OK);
	EXPECT(size, 1);

	EXPECT(cfs_write(file, "a", 1), CERR_DESC);
	EXPECT(cfs_read(NULL, buf, 1), CERR_VAL);
	EXPECT(cfs_read(file, NULL, 1), CERR_VAL);
	EXPECT(cfs_read(file, buf, 0), CERR_OK);
	EXPECT(cfs_read(file, buf, 1), CERR_OK);
	EXPECT(buf[0], 'a');

	EXPECT(cfs_close(file), CERR_OK);

	EXPECT(cfs_rmdir(TEST_DIR), CERR_NOT_EMPTY);
	EXPECT(cfs_rmdir(TEST_DIR "/" TEST_FILE), CERR_NOT_FOUND);
	EXPECT(cfs_rmfile(TEST_DIR "/" TEST_FILE), CERR_OK);
	EXPECT(cfs_rmfile(TEST_DIR), CERR_NOT_FOUND);
	EXPECT(cfs_rmdir(TEST_DIR), CERR_OK);

	EXPECT(cfs_getcwd(NULL, 1), CERR_VAL);
	EXPECT(cfs_getcwd(buf, 0), CERR_VAL);
	EXPECT(cfs_getcwd(buf, 1), CERR_MEM);
	EXPECT(cfs_getcwd(buf, sizeof(buf)), CERR_OK);
#ifdef C_WIN
	EXPECT(cfs_unlink(TEST_FILE), CERR_UNSUPPORTED);
#endif

	return ret;
}

static int t_cfs_ls()
{
	int ret = 0;

	char buf[256] = {0};
	void *it;

	EXPECT(cfs_lsdir_begin(NULL, &it, buf, sizeof(buf)), CERR_VAL);
	EXPECT(cfs_lsdir_begin("", &it, buf, sizeof(buf)), CERR_NOT_FOUND);
	EXPECT(cfs_lsdir_begin(TEST_DIR, NULL, buf, sizeof(buf)), CERR_VAL);
	EXPECT(cfs_lsdir_begin(TEST_DIR, &it, NULL, sizeof(buf)), CERR_VAL);
	EXPECT(cfs_lsdir_begin(TEST_DIR, &it, buf, sizeof(buf)), CERR_NOT_FOUND);
	EXPECT(cfs_lsdir_begin(TEST_DIR "/" TEST_DIR, &it, buf, sizeof(buf)), CERR_NOT_FOUND);

	EXPECT(cfs_lsfile_begin(NULL, &it, buf, sizeof(buf)), CERR_VAL);
	EXPECT(cfs_lsfile_begin("", &it, buf, sizeof(buf)), CERR_NOT_FOUND);
	EXPECT(cfs_lsfile_begin(TEST_DIR, NULL, buf, sizeof(buf)), CERR_VAL);
	EXPECT(cfs_lsfile_begin(TEST_DIR, &it, NULL, sizeof(buf)), CERR_VAL);
	EXPECT(cfs_lsfile_begin(TEST_DIR, &it, buf, sizeof(buf)), CERR_NOT_FOUND);
	EXPECT(cfs_lsfile_begin(TEST_DIR "/" TEST_DIR, &it, buf, sizeof(buf)), CERR_NOT_FOUND);

	cfs_mkdir(TEST_DIR);

	EXPECT(cfs_lsdir_begin(TEST_DIR, &it, buf, sizeof(buf)), CERR_END);
	EXPECT(cfs_lsdir_end(NULL), CERR_VAL);
	EXPECT(cfs_lsdir_end(&it), CERR_OK);

	EXPECT(cfs_lsfile_begin(TEST_DIR, &it, buf, sizeof(buf)), CERR_END);
	EXPECT(cfs_lsfile_end(NULL), CERR_VAL);
	EXPECT(cfs_lsfile_end(&it), CERR_OK);

	cfs_mkfile(TEST_DIR "/file0");
	cfs_mkfile(TEST_DIR "/file1");
	cfs_mkfile(TEST_DIR "/file2");

	EXPECT(cfs_lsdir_begin(TEST_DIR "/file0", &it, buf, sizeof(buf)), CERR_TYPE);
	EXPECT(cfs_lsdir_begin(TEST_DIR, &it, buf, sizeof(buf)), CERR_END);
	EXPECT(cfs_lsdir_end(&it), CERR_OK);

	cfs_mkdir(TEST_DIR "/dir0");
	cfs_mkdir(TEST_DIR "/dir1");
	cfs_mkdir(TEST_DIR "/dir2");

	EXPECT(cfs_lsdir_begin(TEST_DIR, &it, buf, sizeof(buf)), CERR_OK);
	EXPECT(cfs_lsdir_next(NULL, buf, sizeof(buf)), CERR_VAL);
	EXPECT(cfs_lsdir_next(&it, NULL, sizeof(buf)), CERR_VAL);
	EXPECT(cfs_lsdir_next(&it, buf, 0), CERR_MEM);
	EXPECT(cfs_lsdir_next(&it, buf, sizeof(buf)), CERR_OK);
	EXPECT(cfs_lsdir_next(&it, buf, sizeof(buf)), CERR_END);
	EXPECT(cfs_lsdir_end(&it), CERR_OK);

	int dirs = 0;
	cerr_t i;
	cfs_lsdir_foreach(i, TEST_DIR, &it, buf, sizeof(buf))
	{
		dirs++;
	}
	EXPECT(dirs, 3);

	EXPECT(cfs_lsfile_begin(TEST_DIR "/file0", &it, buf, sizeof(buf)), CERR_TYPE);
	EXPECT(cfs_lsfile_begin(TEST_DIR, &it, buf, sizeof(buf)), CERR_OK);
	EXPECT(cfs_lsfile_next(NULL, buf, sizeof(buf)), CERR_VAL);
	EXPECT(cfs_lsfile_next(&it, NULL, sizeof(buf)), CERR_VAL);
	EXPECT(cfs_lsfile_next(&it, buf, 0), CERR_MEM);
	EXPECT(cfs_lsfile_next(&it, buf, sizeof(buf)), CERR_OK);
	EXPECT(cfs_lsfile_next(&it, buf, sizeof(buf)), CERR_END);
	EXPECT(cfs_lsfile_end(&it), CERR_OK);

	int files = 0;
	cfs_lsfile_foreach(i, TEST_DIR, &it, buf, sizeof(buf))
	{
		files++;
	}
	EXPECT(files, 3);

	cfs_rmfile(TEST_DIR "/file0");
	cfs_rmfile(TEST_DIR "/file1");
	cfs_rmfile(TEST_DIR "/file2");

	EXPECT(cfs_lsfile_begin(TEST_DIR, &it, buf, sizeof(buf)), CERR_END);
	EXPECT(cfs_lsfile_end(&it), CERR_OK);

	cfs_rmdir(TEST_DIR "/dir0");
	cfs_rmdir(TEST_DIR "/dir1");
	cfs_rmdir(TEST_DIR "/dir2");

	cfs_rmdir(TEST_DIR);

	return ret;
}

static int t_csock()
{
	int ret = 0;

#if defined(C_LINUX)
	void *server, *client, *peer, *other, *refused_server, *refused_client, *bad;
	char path[108];
	char refused_path[108];
	u8 buf[8] = {0};
	size_t n;
	int flags;

	bad = (void *)-1;

	c_sprintf(path, sizeof(path), 0, "/tmp/cbase_%ld.sock", (long)cproc_getpid());
	c_sprintf(refused_path, sizeof(refused_path), 0, "/tmp/cbase_refused_%ld.sock", (long)cproc_getpid());
	cfs_unlink(path);
	cfs_unlink(refused_path);

	EXPECT(csock_open(-1, -1, -1, NULL), CERR_VAL);
	EXPECT(csock_open(-1, -1, -1, &server), CERR_VAL);
	EXPECT(csock_open(CSOCK_FAMILY_UNIX, -1, -1, &server), CERR_VAL);
	EXPECT(csock_open(CSOCK_FAMILY_UNIX, CSOCK_TYPE_STREAM, -1, &server), CERR_PROTO);
	EXPECT(csock_open(CSOCK_FAMILY_UNIX, CSOCK_TYPE_STREAM, 0, &server), CERR_OK);
	EXPECT(csock_open(CSOCK_FAMILY_UNIX, CSOCK_TYPE_STREAM, 0, &client), CERR_OK);
	EXPECT(csock_open(CSOCK_FAMILY_UNIX, CSOCK_TYPE_STREAM, 0, &other), CERR_OK);
	EXPECT(csock_accept(other, &peer), CERR_STATE);
	EXPECT(csock_read(other, buf, 1, &n), CERR_STATE);
	EXPECT(csock_write(other, buf, 1, &n), CERR_CONN);
	EXPECT(csock_close(other), CERR_OK);

	EXPECT(csock_bind(server, -1, NULL, 0), CERR_VAL);
	EXPECT(csock_bind(server, -1, "", 0), CERR_VAL);
	EXPECT(csock_bind(server, CSOCK_FAMILY_UNIX, path, sizeof(path) + 1), CERR_VAL);
	EXPECT(csock_bind(bad, CSOCK_FAMILY_UNIX, path, cstrlen(path) + 1), CERR_DESC);
	EXPECT(csock_bind(server, CSOCK_FAMILY_UNIX, path, cstrlen(path) + 1), CERR_OK);
	EXPECT(csock_open(CSOCK_FAMILY_UNIX, CSOCK_TYPE_STREAM, 0, &other), CERR_OK);
	EXPECT(csock_bind(other, CSOCK_FAMILY_UNIX, path, cstrlen(path) + 1), CERR_EXIST);
	EXPECT(csock_close(other), CERR_OK);

	EXPECT(csock_listen(NULL, 0), CERR_VAL);
	EXPECT(csock_listen(bad, 1), CERR_DESC);
	EXPECT(csock_listen(server, 1), CERR_OK);
	EXPECT(csock_read(server, buf, 1, &n), CERR_STATE);
	EXPECT(csock_get_flags(server, &flags), CERR_OK);
	EXPECT(csock_set_flags(server, flags | 04000), CERR_OK);
	EXPECT(csock_accept(server, &peer), CERR_AGAIN);
	EXPECT(csock_set_flags(server, flags), CERR_OK);

	EXPECT(csock_connect(client, -1, NULL, 0), CERR_VAL);
	EXPECT(csock_connect(client, -1, "", 0), CERR_VAL);
	EXPECT(csock_connect(client, CSOCK_FAMILY_UNIX, path, sizeof(path) + 1), CERR_VAL);
	EXPECT(csock_connect(bad, CSOCK_FAMILY_UNIX, path, cstrlen(path) + 1), CERR_DESC);
	EXPECT(csock_open(CSOCK_FAMILY_UNIX, CSOCK_TYPE_STREAM, 0, &refused_client), CERR_OK);
	EXPECT(csock_connect(refused_client, CSOCK_FAMILY_UNIX, refused_path, cstrlen(refused_path) + 1), CERR_NOT_FOUND);
	EXPECT(csock_close(refused_client), CERR_OK);
	EXPECT(csock_open(CSOCK_FAMILY_UNIX, CSOCK_TYPE_STREAM, 0, &refused_server), CERR_OK);
	EXPECT(csock_bind(refused_server, CSOCK_FAMILY_UNIX, refused_path, cstrlen(refused_path) + 1), CERR_OK);
	EXPECT(csock_close(refused_server), CERR_OK);
	EXPECT(csock_open(CSOCK_FAMILY_UNIX, CSOCK_TYPE_STREAM, 0, &refused_client), CERR_OK);
	EXPECT(csock_connect(refused_client, CSOCK_FAMILY_UNIX, refused_path, cstrlen(refused_path) + 1), CERR_CONN);
	EXPECT(csock_close(refused_client), CERR_OK);
	cfs_unlink(refused_path);
	EXPECT(csock_connect(client, CSOCK_FAMILY_UNIX, path, cstrlen(path) + 1), CERR_OK);

	EXPECT(csock_accept(NULL, NULL), CERR_VAL);
	EXPECT(csock_accept(bad, &peer), CERR_DESC);
	EXPECT(csock_accept(server, &peer), CERR_OK);

	EXPECT(csock_write(NULL, NULL, 0, NULL), CERR_VAL);
	EXPECT(csock_write(bad, buf, 1, &n), CERR_DESC);
	buf[0] = 0x12;
	n      = 0;
	EXPECT(csock_write(client, buf, 1, &n), CERR_OK);
	EXPECT(n, 1);

	EXPECT(csock_read(NULL, NULL, 0, NULL), CERR_VAL);
	EXPECT(csock_read(bad, buf, 1, &n), CERR_DESC);
	buf[0] = 0;
	n      = 0;
	EXPECT(csock_read(peer, buf, 1, &n), CERR_OK);
	EXPECT(n, 1);
	EXPECT(buf[0], 0x12);

	EXPECT(t_set_alarm(1), 0);
	EXPECT(csock_read(peer, buf, 1, &n), CERR_INTERRUPT);

	csock_get_flags(peer, &flags);
	EXPECT(flags >= 0, 1);
	EXPECT(csock_set_flags(peer, flags | 04000), CERR_OK);
	EXPECT(csock_read(peer, buf, 1, &n), CERR_AGAIN);
	EXPECT(csock_set_flags(peer, flags), CERR_OK);

	EXPECT(csock_setopt(NULL, CSOCK_OPT_SNDBUF, NULL, 0), CERR_VAL);
	int size = 4096;
	EXPECT(csock_setopt(client, CSOCK_OPT_SNDBUF, NULL, sizeof(size)), CERR_VAL);
	EXPECT(csock_setopt(bad, -1, &size, sizeof(size)), CERR_VAL);
	EXPECT(csock_setopt(bad, CSOCK_OPT_SNDBUF, &size, sizeof(size)), CERR_DESC);
	EXPECT(csock_setopt(client, CSOCK_OPT_SNDBUF, &size, sizeof(size)), CERR_OK);

	EXPECT(csock_get_flags(NULL, NULL), CERR_VAL);
	EXPECT(csock_get_flags(bad, &flags), CERR_DESC);

	EXPECT(csock_set_flags(NULL, 0), CERR_VAL);
	EXPECT(csock_set_flags(bad, 0), CERR_DESC);

	EXPECT(t_fill_socket(client), 0);
	EXPECT(t_set_alarm(1), 0);
	EXPECT(csock_write(client, buf, 1, &n), CERR_INTERRUPT);

	EXPECT(csock_close(NULL), CERR_VAL);
	EXPECT(csock_close(bad), CERR_DESC);
	EXPECT(csock_close(peer), CERR_OK);
	EXPECT(csock_close(client), CERR_OK);
	EXPECT(csock_close(server), CERR_OK);
	cfs_unlink(path);

#else
	void *sock = (void *)1;
	void *peer;
	int flags = 0;
	int size = 4096;
	u8 buf[1] = {0};
	size_t n;

	EXPECT(csock_open(CSOCK_FAMILY_UNIX, CSOCK_TYPE_STREAM, 0, NULL), CERR_VAL);
	EXPECT(csock_open(CSOCK_FAMILY_UNIX, CSOCK_TYPE_STREAM, 0, &sock), CERR_UNSUPPORTED);
	EXPECT(csock_close(NULL), CERR_VAL);
	EXPECT(csock_close(sock), CERR_UNSUPPORTED);
	EXPECT(csock_setopt(NULL, CSOCK_OPT_SNDBUF, &size, sizeof(size)), CERR_VAL);
	EXPECT(csock_setopt(sock, CSOCK_OPT_SNDBUF, NULL, sizeof(size)), CERR_VAL);
	EXPECT(csock_setopt(sock, CSOCK_OPT_SNDBUF, &size, sizeof(size)), CERR_UNSUPPORTED);
	EXPECT(csock_get_flags(NULL, &flags), CERR_VAL);
	EXPECT(csock_get_flags(sock, NULL), CERR_VAL);
	EXPECT(csock_get_flags(sock, &flags), CERR_UNSUPPORTED);
	EXPECT(csock_set_flags(NULL, flags), CERR_VAL);
	EXPECT(csock_set_flags(sock, flags), CERR_UNSUPPORTED);
	EXPECT(csock_bind(NULL, CSOCK_FAMILY_UNIX, "sock", 5), CERR_VAL);
	EXPECT(csock_bind(sock, CSOCK_FAMILY_UNIX, NULL, 5), CERR_VAL);
	EXPECT(csock_bind(sock, CSOCK_FAMILY_UNIX, "sock", 5), CERR_UNSUPPORTED);
	EXPECT(csock_listen(NULL, 1), CERR_VAL);
	EXPECT(csock_listen(sock, 1), CERR_UNSUPPORTED);
	EXPECT(csock_connect(NULL, CSOCK_FAMILY_UNIX, "sock", 5), CERR_VAL);
	EXPECT(csock_connect(sock, CSOCK_FAMILY_UNIX, NULL, 5), CERR_VAL);
	EXPECT(csock_connect(sock, CSOCK_FAMILY_UNIX, "sock", 5), CERR_UNSUPPORTED);
	EXPECT(csock_accept(NULL, &peer), CERR_VAL);
	EXPECT(csock_accept(sock, NULL), CERR_VAL);
	EXPECT(csock_accept(sock, &peer), CERR_UNSUPPORTED);
	EXPECT(csock_write(NULL, buf, sizeof(buf), &n), CERR_VAL);
	EXPECT(csock_write(sock, NULL, sizeof(buf), &n), CERR_VAL);
	EXPECT(csock_write(sock, buf, sizeof(buf), &n), CERR_UNSUPPORTED);
	EXPECT(csock_read(NULL, buf, sizeof(buf), &n), CERR_VAL);
	EXPECT(csock_read(sock, NULL, sizeof(buf), &n), CERR_VAL);
	EXPECT(csock_read(sock, buf, sizeof(buf), &n), CERR_UNSUPPORTED);
#endif

	return ret;
}

static int t_ctime_sleep()
{
	int ret = 0;

	u64 start, end;

	start = c_time();

	c_sleep(1000);
#ifdef C_WIN
	EXPECT(c_timer(1), 1);
#endif

	end = c_time();

	u64 eps = end - start - 1000;

	EXPECT(eps < 162, 1);

	return ret;
}

static int t_ctime_str()
{
	int ret = 0;

	char buf[CTIME_BUF_SIZE] = {0};

	c_time_str(NULL);
	c_time_str(buf);

	EXPECT(buf[4], '-');
	EXPECT(buf[7], '-');
	EXPECT(buf[10], ' ');
	EXPECT(buf[13], ':');
	EXPECT(buf[16], ':');
	EXPECT(buf[19], '.');

	return ret;
}

static int t_dst()
{
	int ret	    = 0;
	char buf[2] = {0};

	EXPECT(dputs(DST_NONE(), STRV_NULL), 0);
	EXPECT(dputf(DST_NONE(), NULL), 0);

	EXPECT(dputs(DST_BUF(buf), STRV_NULL), 0);
	EXPECT(dputs(DST_BUF(buf), STRV("a")), 1);
	EXPECT(buf[0], 'a');
	EXPECT(dputs(DST_BUF(buf), STRV("bb")), 0);
	EXPECT(dputs(DST_BUF(buf), STRV("ccc")), 0);

	EXPECT(dputf(DST_BUF(buf), NULL), 0);
	EXPECT(dputf(DST_BUF(buf), "d"), 1);
	EXPECT(buf[0], 'd');
#ifdef C_LINUX
	EXPECT(dputf(DST_BUF(buf), "ee"), 2);
	EXPECT(dputf(DST_BUF(buf), "fff"), 0);
#endif

	return ret;
}

static int t_print()
{
	int ret	    = 0;
	char buf[1] = {0};

	EXPECT(c_printf(NULL), -1);

	EXPECT(c_sprintf(NULL, 0, 0, ""), 0);
	EXPECT(c_sprintf(buf, sizeof(buf), 0, ""), 0);
	EXPECT(c_sprintv(NULL, 0, 0, NULL, NULL), -1);

	return ret;
}

static int t_wdst()
{
	int ret	       = 0;
	wchar_t buf[2] = {0};

	EXPECT(wdputs(WDST_NONE(), WSTRV_NULL), 0);
	EXPECT(wdputf(WDST_NONE(), NULL), 0);

	EXPECT(wdputs(WDST_BUF(buf), WSTRV_NULL), 0);
	EXPECT(wdputs(WDST_BUF(buf), WSTRV(L"a")), 1);
	EXPECT(buf[0], L'a');
	EXPECT(wdputs(WDST_BUF(buf), WSTRV(L"bb")), 0);
	EXPECT(wdputs(WDST_BUF(buf), WSTRV(L"ccc")), 0);

	EXPECT(wdputf(WDST_BUF(buf), NULL), 0);
	EXPECT(wdputf(WDST_BUF(buf), L"d"), 1);
	EXPECT(buf[0], 'd');
#ifdef C_LINUX
	EXPECT(wdputf(WDST_BUF(buf), L"ee"), 0);
	EXPECT(wdputf(WDST_BUF(buf), L"fff"), 0);
#endif

	return ret;
}

static int t_wprint()
{
	int ret	       = 0;
	wchar_t buf[1] = {0};

	EXPECT(c_wprintf(NULL), -1);

	EXPECT(c_swprintf(buf, sizeof(buf), 0, L""), 0);
	EXPECT(c_swprintv(NULL, 0, 0, NULL, NULL), -1);

	return ret;
}

static int t_char()
{
	int ret = 0;

	EXPECT(dputs(DST_STD(), STRV("┌─┬─┐")), 5 * 3);
	c_startw(stdout);
	EXPECT(wdputs(WDST_STD(), WSTRV(L"\u250C\u2500\u252C\u2500\u2510")), 5);
	c_endw(stdout);
	c_printf("\n");

	EXPECT(dputf(DST_STD(), "├─┼─┤"), 5 * 3);
	c_startw(stdout);
	EXPECT(wdputf(WDST_STD(), L"\u251C\u2500\u253C\u2500\u2524"), 5);
	c_endw(stdout);
	c_printf("\n");

	EXPECT(dputf(DST_STD(), "└─┴─┘"), 5 * 3);
	c_startw(stdout);
	EXPECT(wdputf(WDST_STD(), L"\u2514\u2500\u2534\u2500\u2518"), 5);
	c_endw(stdout);
	c_printf("\n");

	return ret;
}

static int t_type()
{
	int ret = 0;

	EXPECT(sizeof(S8_MIN), 1);
	EXPECT(sizeof(S8_MAX), 1);
	EXPECT(sizeof(U8_MIN), 1);
	EXPECT(sizeof(U8_MAX), 1);
	EXPECT(sizeof(S16_MIN), 2);
	EXPECT(sizeof(S16_MAX), 2);
	EXPECT(sizeof(U16_MIN), 2);
	EXPECT(sizeof(U16_MAX), 2);
	EXPECT(sizeof(S32_MIN), 4);
	EXPECT(sizeof(S32_MAX), 4);
	EXPECT(sizeof(U32_MIN), 4);
	EXPECT(sizeof(U32_MAX), 4);
	EXPECT(sizeof(S64_MIN), 8);
	EXPECT(sizeof(S64_MAX), 8);
	EXPECT(sizeof(U64_MIN), 8);
	EXPECT(sizeof(U64_MAX), 8);

	return ret;
}

int main()
{
	int ret = 0;

	c_print_init();

	EXPECT(t_mem_stats(), 0);
	EXPECT(t_cerr(), 0);
	EXPECT(t_cproc(), 0);
	EXPECT(t_cfs(), 0);
	EXPECT(t_cfs_ls(), 0);
	EXPECT(t_csock(), 0);
	EXPECT(t_ctime_sleep(), 0);
	EXPECT(t_ctime_str(), 0);
	EXPECT(t_dst(), 0);
	EXPECT(t_print(), 0);
	EXPECT(t_wdst(), 0);
	EXPECT(t_wprint(), 0);
	EXPECT(t_char(), 0);
	EXPECT(t_type(), 0);

	return ret;
}
