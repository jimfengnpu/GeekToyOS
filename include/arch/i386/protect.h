#ifndef X86_PROTECT_H
#define X86_PROTECT_H
#include <kernel/types.h>
// 各个内存段所在全局描述符表下标
#define SEG_NULL     0
#define SEG_KTEXT    1
#define SEG_KDATA    2
#define SEG_UTEXT    3
#define SEG_UDATA    4
#define SEG_TSS      5

#define GD_KTEXT    ((SEG_KTEXT) << 3)      // 代码段
#define GD_KDATA    ((SEG_KDATA) << 3)      // 数据段
#define GD_UTEXT    ((SEG_UTEXT) << 3)      // 代码段
#define GD_UDATA    ((SEG_UDATA) << 3)      // 数据段
#define GD_TSS      ((SEG_TSS) << 3)       // 任务段

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
#define KERNEL_CS   ((GD_KTEXT) | PL_KERNEL)
#define KERNEL_DS   ((GD_KDATA) | PL_KERNEL)
#define USER_CS     ((GD_UTEXT) | PL_USER)
#define USER_DS     ((GD_UDATA) | PL_USER)

// TSS 描述符
typedef struct tss_entry_s {
        u32 ts_link;         // old ts selector
        u32 ts_esp0;         // stack pointers and segment selectors
        u16 ts_ss0;          // after an increase in privilege level
        u16 ts_padding1;
        u32 ts_esp1;
        u16 ts_ss1;
        u16 ts_padding2;
        u32 ts_esp2;
        u16 ts_ss2;
        u16 ts_padding3;
        u32 ts_cr3;          // page directory base
        u32 ts_eip;          // saved state from last task switch
        u32 ts_eflags;
        u32 ts_eax;          // more saved state (registers)
        u32 ts_ecx;
        u32 ts_edx;
        u32 ts_ebx;
        u32 ts_esp;
        u32 ts_ebp;
        u32 ts_esi;
        u32 ts_edi;
        u16 ts_es;           // even more saved state (segment selectors)
        u16 ts_padding4;
        u16 ts_cs;
        u16 ts_padding5;
        u16 ts_ss;
        u16 ts_padding6;
        u16 ts_ds;
        u16 ts_padding7;
        u16 ts_fs;
        u16 ts_padding8;
        u16 ts_gs;
        u16 ts_padding9;
        u16 ts_ldt;
        u16 ts_padding10;
        u16 ts_t;            // trap on task switch
        u16 ts_iomb;         // i/o map base address
} __attribute__((packed)) tss_t;

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
    u32 base;
} __attribute__((packed)) gdt_ptr;

/* 门描述符 */
typedef struct gate
{
	u16	offset_low;	/* Offset Low */
	u16	selector;	/* Selector */
	u8	dcount;		/* 该字段只在调用门描述符中有效。
				如果在利用调用门调用子程序时引起特权级的转换和堆栈的改变，需要将外层堆栈中的参数复制到内层堆栈。
				该双字计数字段就是用于说明这种情况发生时，要复制的双字参数的数量。 */
	u8	attr;		/* P(1) DPL(2) DT(1) TYPE(4) */
	u16	offset_high;	/* Offset High */
}gate_t;
#endif