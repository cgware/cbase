#include "mem_stats.h"
#include "platform.h"
#include "print.h"
#include "wprint.h"

#include <stdio.h>

#define EXPECT(_check)                                                                                                                     \
	if (!(_check)) {                                                                                                                   \
		printf("\033[31m%s\033[0m\n", #_check);                                                                                    \
		ret = 1;                                                                                                                   \
	}

static int t_mem_stats()
{
	int ret = 0;

	const mem_stats_t *stats;
	mem_stats_t tmp = {0};

	stats = mem_stats_get();

	mem_stats_set(NULL);
	mem_stats_alloc(1);
	EXPECT(tmp.mem == 0);
	mem_stats_realloc(1, 2);
	EXPECT(tmp.mem == 0);
	mem_stats_free(2);
	EXPECT(tmp.mem == 0);

	EXPECT(mem_stats_set(&tmp) == stats);
	mem_stats_alloc(1);
	EXPECT(tmp.mem == 1);
	mem_stats_realloc(1, 2);
	EXPECT(tmp.mem == 2);
	mem_stats_free(2);
	EXPECT(tmp.mem == 0);

	mem_stats_set((mem_stats_t *)stats);

	return ret;
}

static int t_print()
{
	int ret	    = 0;
	char buf[1] = {0};

	EXPECT(c_printf(NULL) == 0);

	EXPECT(c_sprintf(buf, sizeof(buf), 0, "") == 0);
	EXPECT(c_sprintv(NULL, 0, 0, NULL, NULL) == 0);
	EXPECT(c_dprintf(PRINT_DST_NONE(), NULL) == 0);
#ifdef C_LINUX
	char cbuf[2] = {0};
	EXPECT(c_dprintf(PRINT_DST_BUF(cbuf, sizeof(cbuf), 0), "abc") == 0);
#endif

	return ret;
}

static int t_wprint()
{
	int ret	       = 0;
	wchar_t buf[1] = {0};

	EXPECT(c_wprintf(NULL) == 0);

	EXPECT(c_swprintf(buf, sizeof(buf), 0, L"") == 0);
	EXPECT(c_swprintv(NULL, 0, 0, NULL, NULL) == 0);
	EXPECT(c_dwprintf(PRINT_DST_WNONE(), NULL) == 0);
#ifdef C_LINUX
	wchar_t cbuf[2] = {0};
	EXPECT(c_dwprintf(PRINT_DST_WBUF(cbuf, sizeof(cbuf), 0), L"abc") == 0);
#endif

	return ret;
}

static int t_char()
{
	int ret = 0;

	EXPECT(c_dprintf(PRINT_DST_STD(), "┌─┬─┐") == 5 * 3);
	c_startw(stdout);
	EXPECT(c_dwprintf(PRINT_DST_WSTD(), L"\u250C\u2500\u252C\u2500\u2510") == 5);
	c_endw(stdout);
	c_printf("\n");

	EXPECT(c_dprintf(PRINT_DST_STD(), "├─┼─┤") == 5 * 3);
	c_startw(stdout);
	EXPECT(c_dwprintf(PRINT_DST_WSTD(), L"\u251C\u2500\u253C\u2500\u2524") == 5);
	c_endw(stdout);
	c_printf("\n");

	EXPECT(c_dprintf(PRINT_DST_STD(), "└─┴─┘") == 5 * 3);
	c_startw(stdout);
	EXPECT(c_dwprintf(PRINT_DST_WSTD(), L"\u2514\u2500\u2534\u2500\u2518") == 5);
	c_endw(stdout);
	c_printf("\n");

	return ret;
}

int main()
{
	int ret = 0;

	c_print_init();

	EXPECT(t_mem_stats() == 0);
	EXPECT(t_print() == 0);
	EXPECT(t_wprint() == 0);
	EXPECT(t_char() == 0);

	return ret;
}
