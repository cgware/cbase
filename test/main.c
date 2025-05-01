#include "cfs.h"
#include "ctime.h"
#include "dst.h"
#include "mem_stats.h"
#include "platform.h"
#include "print.h"
#include "type.h"
#include "wdst.h"
#include "wprint.h"

#include <stdio.h>

#define EXPECT(_act, _exp)                                                                                                                 \
	do {                                                                                                                               \
		unsigned long _actual = (unsigned long)(_act);                                                                             \
		unsigned long _expect = (unsigned long)(_exp);                                                                             \
		if (_actual != _expect) {                                                                                                  \
			printf("\033[31m%s:%d: %lX != %lX\033[0m\n", __FILE__, __LINE__, _actual, _expect);                                \
			ret = 1;                                                                                                           \
		}                                                                                                                          \
	} while (0)

#define TEST_DIR  "t_cfs_tmp"
#define TEST_FILE "t_cfs_file.txt"

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

static int t_ctime_sleep()
{
	int ret = 0;

	u64 start, end;

	start = c_time();

	c_sleep(1000);

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
	EXPECT(t_cfs(), 0);
	EXPECT(t_cfs_ls(), 0);
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
