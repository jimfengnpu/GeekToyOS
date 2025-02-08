#ifndef SPINLOCK_H
#define SPINLOCK_H
#include <lib/types.h>

struct spinlock{
    u32 lock;
};

void acquire(struct spinlock *lock);
void release(struct spinlock *lock);
#endif