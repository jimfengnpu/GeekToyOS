#ifndef X86_CPU_H
#define X86_CPU_H
#include <lib/types.h>


#define DEFINE_IO(bwl, bw, type)						\
static inline void out##bwl(int port, type value)		\
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

static inline void pause(void)
{
	__builtin_ia32_pause();
}

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

static inline void interrupt_enable()
{
	asm volatile ("sti");
}

static inline void interrupt_disable()
{
	asm volatile ("cli");
}

// enum{

// };

/** issue a single request to CPUID. Fits 'intel features', for instance
 *  note that even if only "eax" and "edx" are of interest, other registers
 *  will be modified by the operation, so we need to tell the compiler about it.
 */
static inline void cpuid(int code, u32 *a, u32 *d) {
	asm volatile("cpuid":"=a"(*a),"=d"(*d):"a"(code):"ecx","ebx");
}
    
/** issue a complete request, storing general registers output as a string
 */
static inline void cpuid_string(int code, char where[16]) {
	asm volatile("cpuid":"=a"(*where),"=b"(*(where+4)),
               "=c"(*(where+8)),"=d"(*(where+12)):"a"(code));
}
#endif