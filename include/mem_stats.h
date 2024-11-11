#ifndef MEM_STATS_H
#define MEM_STATS_H

#include <stddef.h>

typedef struct mem_stats_s {
	size_t mem;
	size_t peak;
	size_t total;
	size_t allocs;
	size_t reallocs;
} mem_stats_t;

const mem_stats_t *mem_stats_get();
const mem_stats_t *mem_stats_set(mem_stats_t *stats);

void mem_stats_alloc(size_t size);
void mem_stats_realloc(size_t old_size, size_t new_size);
void mem_stats_free(size_t size);

#endif
