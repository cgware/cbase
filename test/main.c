#include "mem_stats.h"
#include "platform.h"
#include "print.h"

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

	EXPECT(c_printv(NULL, NULL) == 0);

	EXPECT(c_sprintf(buf, sizeof(buf), 0, "") == 0);
	EXPECT(c_sprintv(NULL, 0, 0, NULL, NULL) == 0);
	EXPECT(c_dprintf(PRINT_DST_NONE(), NULL) == 0);
#ifdef C_LINUX
	char cbuf[2] = {0};
	EXPECT(c_dprintf(PRINT_DST_BUF(cbuf, sizeof(cbuf), 0), "abc") == 0);
#endif

	return ret;
}

static int t_char()
{
	int ret = 0;

	EXPECT(c_dprintf(PRINT_DST_STD(), "┌─┬─┐") == 5 * 3);
	c_printf("\n");

	EXPECT(c_dprintf(PRINT_DST_STD(), "├─┼─┤") == 5 * 3);
	c_printf("\n");

	EXPECT(c_dprintf(PRINT_DST_STD(), "└─┴─┘") == 5 * 3);
	c_printf("\n");

	return ret;
}

int main()
{
	int ret = 0;

	EXPECT(t_mem_stats() == 0);
	EXPECT(t_print() == 0);
	EXPECT(t_char() == 0);

	return ret;
}
