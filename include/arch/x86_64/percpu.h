#pragma once
#include <kernel/kernel.h>
#include <kernel/sched.h>

struct percpu {
    size_t  id;
    struct task* current_task;
    struct task *idle_task;
    struct runq *runqueue;
    struct spinlock preempt_lock;
    int atomic_preempt_count;
};

#define __percpu(var) (((struct percpu *)NULL)->var)
#define __percpu_type(var) typeof(__percpu(var))
#define __percpu_marker(var)	((volatile __percpu_type(var) *)&__percpu(var))

#define percpu_get(var) ({						\
	__percpu_type(var) res;						\
	asm ("mov %%gs:%1, %0"  					\
	     : "=r" (res)						\
	     : "m" (*__percpu_marker(var))                              \
	);                                                              \
	res; })

#define __percpu_set(suffix, var, val)					\
({									\
	asm ("mov" suffix " %1, %%gs:%0"				\
	     : "=m" (*__percpu_marker(var))				\
	     : "ir" (val));						\
})

#define percpu_set(var, val)						\
({									\
	switch (sizeof(__percpu_type(var))) {				\
	case 1: __percpu_set("b", var, val); break;			\
	case 2: __percpu_set("w", var, val); break;			\
	case 4: __percpu_set("l", var, val); break;			\
	case 8: __percpu_set("q", var, val); break;			\
	default: panic("invalid percpu set\n");				\
	}								\
}) 

#define current (percpu_get(current_task))

extern struct percpu *percpu_table[];

static inline struct percpu *percpu_id(u8 id)
{
	return percpu_table[id];
}

static inline struct percpu *percpu_self(void)
{
	return percpu_table[percpu_get(id)];
}

#define thiscpu (percpu_self())