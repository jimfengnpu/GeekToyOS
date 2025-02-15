#ifndef X64_PROTECT_H
#define X64_PROTECT_H
#include <lib/types.h>
// 各个内存段所在全局描述符表下标
#define SEG_NULL     0
#define SEG_KTEXT    1
#define SEG_KDATA    2
#define SEG_UTEXT32  3
#define SEG_UTEXT64  4
#define SEG_UDATA    5
#define SEG_TSS      6

#define GD_SEL(seg) ((seg) << 3)

/* 描述符类型值说明 */
#define	DA_32			0x4000	/* 32 位段				*/
#define	DA_LIMIT_4K		0x8000	/* 段界限粒度为 4K 字节			*/
#define	DA_DPL0			0x00	/* DPL = 0				*/
#define	DA_DPL1			0x20	/* DPL = 1				*/
#define	DA_DPL2			0x40	/* DPL = 2				*/
#define	DA_DPL3			0x60	/* DPL = 3				*/
/* 存储段描述符类型值说明 */
#define	DA_DR			0x90	/* 存在的只读数据段类型值		*/
#define	DA_DRW			0x92	/* 存在的可读写数据段属性值		*/
#define	DA_DRWA			0x93	/* 存在的已访问可读写数据段类型值	*/
#define	DA_C			0x98	/* 存在的只执行代码段属性值		*/
#define	DA_CR			0x9A	/* 存在的可执行可读代码段属性值		*/
#define	DA_CCO			0x9C	/* 存在的只执行一致代码段属性值		*/
#define	DA_CCOR			0x9E	/* 存在的可执行可读一致代码段属性值	*/
/* 系统段描述符类型值说明 */
#define	DA_LDT			0x82	/* 局部描述符表段类型值			*/
#define	DA_TaskGate		0x85	/* 任务门类型值				*/
#define	DA_386TSS		0x89	/* 可用 386 任务状态段类型值		*/
#define	DA_386CGate		0x8C	/* 386 调用门类型值			*/
#define	DA_386IGate		0x8E	/* 386 中断门类型值			*/
#define	DA_386TGate		0x8F	/* 386 陷阱门类型值			*/

/* 选择子类型值说明 */
/* 其中, SA_ : Selector Attribute */
#define	SA_RPL0		0
#define	SA_RPL1		1
#define	SA_RPL2		2
#define	SA_RPL3		3
#define	SA_TIL		4

// 段描述符 DPL/RPL
#define PL_KERNEL  (SA_RPL0)
#define PL_USER    (SA_RPL3)

// 各个段的全局描述符表的选择子
#define KERNEL_CS   (GD_SEL(SEG_KTEXT) | PL_KERNEL)
#define KERNEL_DS   (GD_SEL(SEG_KDATA) | PL_KERNEL)
#define USER_CS32   (GD_SEL(SEG_UTEXT32) | PL_USER)
#define USER_CS64   (GD_SEL(SEG_UTEXT64) | PL_USER)
#define USER_DS     (GD_SEL(SEG_UDATA) | PL_USER)

#define IST_NONE 0
#define IST_NMI 1
#define IST_DOUBLE_FAULT 2

// TSS 描述符
typedef struct tss_entry32_s {
        u32 link;         // old ts selector
        u32 esp0;         // stack pointers and segment selectors
        u16 ss0;          // after an increase in privilege level
        u16 padding1;
        u32 esp1;
        u16 ss1;
        u16 padding2;
        u32 esp2;
        u16 ss2;
        u16 padding3;
        u32 cr3;          // page directory base
        u32 eip;          // saved state from last task switch
        u32 eflags;
        u32 eax;          // more saved state (registers)
        u32 ecx;
        u32 edx;
        u32 ebx;
        u32 esp;
        u32 ebp;
        u32 esi;
        u32 edi;
        u16 es;           // even more saved state (segment selectors)
        u16 padding4;
        u16 cs;
        u16 padding5;
        u16 ss;
        u16 padding6;
        u16 ds;
        u16 padding7;
        u16 fs;
        u16 padding8;
        u16 gs;
        u16 padding9;
        u16 ldt;
        u16 padding10;
        u16 t;            // trap on task switch
        u16 iomb;         // i/o map base address
} __attribute__((packed)) tss32_t;

// TSS 描述符
typedef struct tss_entry64_s {
    u32 __zero1;
    u64 rsp0;
    u64 rsp1;
    u64 rsp2;
    u64 __zero2;
    u64 ist1;
    u64 ist2;
    u64 ist3;
    u64 ist4;
    u64 ist5;
    u64 ist6;
    u64 ist7;
    u64 __zero3;
    u16 __zero4;
    u16 iomb;
} __attribute__((packed)) tss64_t;

/* 存储段描述符/系统段描述符 */
typedef struct descriptor {
    u16 limit_low;
    u16 base_low;
    u8 base_middle;
    u8 attr1;          /* P(1) DPL(2) DT(1) TYPE(4) */
    u8 limit_high_attr2;  /* G(1) D(1) 0(1) AVL(1) LimitHigh(4) */
    u8 base_high;
} __attribute__((packed)) descriptor_t;

// GDT pointer structure
typedef struct gdt_ptr_s {
    u16 limit;
    addr_t base;
} __attribute__((packed)) gdt_ptr;

#endif