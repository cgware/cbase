#include "cspinlock.h"

#include "platform.h"

#ifdef C_WIN
	#include <windows.h>
#endif

void cspinlock_lock(cspinlock_t *lock)
{
#ifdef C_WIN
	while (InterlockedExchange(&lock->state, 1) != 0) {
	}
#else
	while (__atomic_exchange_n(&lock->state, 1, __ATOMIC_ACQUIRE) != 0) {
	}
#endif
}

void cspinlock_unlock(cspinlock_t *lock)
{
#ifdef C_WIN
	InterlockedExchange(&lock->state, 0);
#else
	__atomic_store_n(&lock->state, 0, __ATOMIC_RELEASE);
#endif
}
