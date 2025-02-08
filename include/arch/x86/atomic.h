#ifndef ATOMIC_H
#define ATOMIC_H

// equ (*ptr)++
static inline int atomic_inc(void *ptr){
    int origin;
	asm volatile(
		"lock\n\t"
        "movl %0, %%ecx\n\t"
        "incl %0"
		:"+m" ((*(int*)ptr)), "=c" (origin):: "memory");
    return origin;
}

// equ --(*ptr)
static inline int atomic_dec_and_test(void *ptr)
{
	unsigned char c;

	asm volatile(
		"lock\n\t"
        "decl %0\n\t"
        "sete %1"
		:"+m" (*((int*)ptr)), "=qm" (c)
		: : "memory");
	return c != 0;
}

static inline int atomic_exchange(void *ptr, int value)
{
	return __atomic_exchange_n((int*)ptr, value, __ATOMIC_SEQ_CST);
}


#endif