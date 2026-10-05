#ifndef CSPINLOCK_H
#define CSPINLOCK_H

typedef struct cspinlock_s {
	volatile long state;
} cspinlock_t;

// clang-format off
#define CSPINLOCK_INIT {0}
// clang-format on

void cspinlock_lock(cspinlock_t *lock);
void cspinlock_unlock(cspinlock_t *lock);

#endif
