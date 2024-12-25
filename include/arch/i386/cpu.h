#ifndef X86_IO_H
#define X86_IO_H
#include <kernel/types.h>

#define DEFINE_IO(bwl, bw, type)						\
static inline void out##bwl(type value, int port)		\
{									\
	asm volatile("out" #bwl " %" #bw "0, %w1"			\
		     : : "a"(value), "Nd"(port));			\
}									\
									\
static inline type in##bwl(int port)				\
{									\
	type value;						\
	asm volatile("in" #bwl " %w1, %" #bw "0"			\
		     : "=a"(value) : "Nd"(port));			\
	return value;							\
}	
DEFINE_IO(b, b, u8)
DEFINE_IO(w, w, u16)
DEFINE_IO(l, , u32)

static inline void halt()
{
    asm volatile ("hlt": : :"memory");
}

// 修改当前页表
static inline void switch_pgd(addr_t pd)
{
    asm volatile ("mov %0, %%cr3" : : "r" (pd));
}

// 通知 CPU 更新页表缓存
static inline void tlb_reload_page(u32 va)
{
    asm volatile ("invlpg (%0)" : : "a" (va));
}

#endif