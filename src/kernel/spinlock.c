#include <kernel/spinlock.h>
#include <atomic.h>

void acquire(struct spinlock *lock)
{
    while(atomic_exchange(&lock->lock, 1) != 0);
}

void release(struct spinlock *lock)
{
    lock->lock = 0;
}