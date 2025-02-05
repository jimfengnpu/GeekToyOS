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

static inline void halt(void)
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

static inline u64 msr_read(u32 msr)
{
	u32 low, high;
	asm volatile (
		"rdmsr"
		: "=a"(low), "=d"(high)
		: "c"(msr)
	);
	return ((u64)high << 32) | low;
}

static inline void msr_write(u32 msr, u64 value)
{
	u32 low = value & 0xFFFFFFFF;
	u32 high = value >> 32;
	asm volatile (
		"wrmsr"
		:
		: "c"(msr), "a"(low), "d"(high)
	);
}

enum{
	// 0x1, edx
    FEAT_X86_FPU          =  0,
    FEAT_X86_VME          =  1,
    FEAT_X86_DE           =  2,
    FEAT_X86_PSE          =  3,
    FEAT_X86_TSC          =  4,
    FEAT_X86_MSR          =  5,
    FEAT_X86_PAE          =  6,
    FEAT_X86_MCE          =  7,
    FEAT_X86_CX8          =  8,
    FEAT_X86_APIC         =  9,
    FEAT_X86_SEP          =  11,
    FEAT_X86_MTRR         =  12,
    FEAT_X86_PGE          =  13,
    FEAT_X86_MCA          =  14,
    FEAT_X86_CMOV         =  15,
    FEAT_X86_PAT          =  16,
    FEAT_X86_PSE36        =  17,
    FEAT_X86_PSN          =  18,
    FEAT_X86_CLFLUSH      =  19,
    FEAT_X86_DS           =  21,
    FEAT_X86_ACPI         =  22,
    FEAT_X86_MMX          =  23,
    FEAT_X86_FXSR         =  24,
    FEAT_X86_SSE          =  25,
    FEAT_X86_SSE2         =  26,
    FEAT_X86_SS           =  27,
    FEAT_X86_HTT          =  28,
    FEAT_X86_TM           =  29,
    FEAT_X86_IA64         =  30,
    FEAT_X86_PBE          =  31,

	// 0x1, ecx
	FEAT_X86_SSE3         =  32,
    FEAT_X86_PCLMUL       =  33,
    FEAT_X86_DTES64       =  34,
    FEAT_X86_MONITOR      =  35,
    FEAT_X86_DS_CPL       =  36,
    FEAT_X86_VMX          =  37,
    FEAT_X86_SMX          =  38,
    FEAT_X86_EST          =  39,
    FEAT_X86_TM2          =  40,
    FEAT_X86_SSSE3        =  41,
    FEAT_X86_CID          =  42,
    FEAT_X86_SDBG         =  43,
    FEAT_X86_FMA          =  44,
    FEAT_X86_CX16         =  45,
    FEAT_X86_XTPR         =  46,
    FEAT_X86_PDCM         =  47,
    FEAT_X86_PCID         =  49,
    FEAT_X86_DCA          =  50,
    FEAT_X86_SSE4_1       =  51,
    FEAT_X86_SSE4_2       =  52,
    FEAT_X86_X2APIC       =  53,
    FEAT_X86_MOVBE        =  54,
    FEAT_X86_POPCNT       =  55,
    FEAT_X86_AES          =  57,
    FEAT_X86_XSAVE        =  58,
    FEAT_X86_OSXSAVE      =  59,
    FEAT_X86_AVX          =  60,
    FEAT_X86_F16C         =  61,
    FEAT_X86_RDRAND       =  62,
    FEAT_X86_HYPERVISOR   =  63,

	// 0x8000_0001, edx（）
	FEAT_X86_SYSCALL	= 64 + 11,
	FEAT_X86_NX			= 64 + 20,
	FEAT_X86_MmxExt		= 64 + 22,
	FEAT_X86_FFXSR		= 64 + 25,
	FEAT_X86_Page1GB	= 64 + 26,
	FEAT_X86_RDTSCP		= 64 + 27,
	FEAT_X86_LM			= 64 + 29,
	FEAT_X86_3DNowExt	= 64 + 30,
	FEAT_X86_3DNow		= 64 + 31,

	// todo 0x8000_0001, ecx
};

struct cpu_feature{
	char manufacture[16];
	u64 feature;
	u64 extend_feature;
};

/** issue a single request to CPUID. Fits 'intel features', for instance
 *  note that even if only "eax" and "edx" are of interest, other registers
 *  will be modified by the operation, so we need to tell the compiler about it.
 */
static inline void cpuid(u32 code, u32 *a, u32 *b, u32 *c, u32 *d) {
	u32 ax, bx, cx, dx;
	asm volatile("cpuid":"=a"(ax),"=b"(bx),"=c"(cx),"=d"(dx):"a"(code):);
	if(a)*a=ax;
	if(b)*b=bx;
	if(c)*c=cx;
	if(d)*d=dx;
}

int check_cpu_feature(int type);

#endif