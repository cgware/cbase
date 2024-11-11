#include "mem_stats.h"

#define MAX(a, b) (a) > (b) ? (a) : (b)

static mem_stats_t *s_stats = NULL;

const mem_stats_t *mem_stats_set(mem_stats_t *stats)
{
	const mem_stats_t *tmp = s_stats;

	s_stats = stats;
	return tmp;
}

const mem_stats_t *mem_stats_get()
{
	return s_stats;
}

void mem_stats_alloc(size_t size)
{
	s_stats->mem += size;
	s_stats->peak = MAX(s_stats->mem, s_stats->peak);
	s_stats->total += size;
	s_stats->allocs++;
}

void mem_stats_realloc(size_t old_size, size_t new_size)
{
	s_stats->mem += new_size - old_size;
	s_stats->peak = MAX(s_stats->mem, s_stats->peak);
	s_stats->total += new_size - old_size;
	s_stats->reallocs++;
}

void mem_stats_free(size_t size)
{
	s_stats->mem -= size;
}
