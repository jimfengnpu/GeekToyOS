#ifndef X86_INTERRUPT_H
#define X86_INTERRUPT_H
#include <lib/types.h>
#include <context.h>
// 中断保存的寄存器类型
typedef struct cpu_context trapframe_t;

#define INTERRUPT_MAX 256
// 中断号定义
#define INT_DIVIDE_ERROR 0
#define INT_DEBUG 1
#define INT_NMI 2
#define INT_BREAKPOINT 3
#define INT_OVERFLOW 4
#define INT_BOUND 5
#define INT_INVALID_OPCODE 6
#define INT_DEVICE_NOT_AVAIL 7
#define INT_DOUBLE_FAULT 8
#define INT_COPROCESSOR 9
#define INT_INVALID_TSS 10
#define INT_SEGMENT 11
#define INT_STACK_FAULT 12
#define INT_GENERAL_PROTECT 13
#define INT_PAGE_FAULT 14

#define INT_X87_FPU 16
#define INT_ALIGNMENT 17
#define INT_MACHINE_CHECK 18
#define INT_SIMD_FLOAT 19

#if ARCH_i386
#define INT_SYSCALL 128
#endif

#define IRQ_BASE   32
#define IRQ_SPURIOUS 0xFF
u8 ioapic_isa_to_gsi(u8 isa);
u8 ioapic_gsi_to_isa(u8 gsi);
#define IRQ_TO_GSI(irq) (ioapic_isa_to_gsi(irq))
#define INT_TO_IRQ(vec) (ioapic_gsi_to_isa((vec) - IRQ_BASE))
#define IRQ_TO_INT(irq) (ioapic_isa_to_gsi(irq) + IRQ_BASE)

#define IRQ_LINT_BASE (IRQ_SPURIOUS - 3) // 252, One for each LINT pin, one for timer
#define IRQ_LINT_TIMER (IRQ_LINT_BASE + 2) // 254
#define IRQ_NMI (IRQ_LINT_BASE - 1) // 251

#define IRQ_IPI_TOP (IRQ_NMI - 1) // 250
#define IRQ_IPI_ABORT (IRQ_IPI_TOP - 0) // 250
#define IRQ_IPI_TLB_SHOOTDOWN (IRQ_IPI_TOP - 1) //249
#define IRQ_IPI_SCHED_HINT (IRQ_IPI_TOP - 2) //248
// // 定义IRQ
// #define IRQ0 32  // 电脑系统计时器
// #define IRQ1 33  // 键盘
// #define IRQ2 34  // 与 IRQ9 相接，MPU-401 MD 使用
// #define IRQ3 35  // 串口设备
// #define IRQ4 36  // 串口设备
// #define IRQ5 37  // 建议声卡使用
// #define IRQ6 38  // 软驱传输控制使用
// #define IRQ7 39  // 打印机传输控制使用
// #define IRQ8 40  // 即时时钟
// #define IRQ9 41  // 与 IRQ2 相接，可设定给其他硬件
// #define IRQ10 42 // 建议网卡使用
// #define IRQ11 43 // 建议 AGP 显卡使用
// #define IRQ12 44 // 接 PS/2 鼠标，也可设定给其他硬件
// #define IRQ13 45 // 协处理器使用
// #define IRQ14 46 // IDE0 传输控制使用
// #define IRQ15 47 // IDE1 传输控制使用

// 初始化中断
void arch_interrupt_init(void);
void interrupt_eoi(u8 irq);
void interrupt_enable_irq(u8 irq);
void interrupt_disable_irq(u8 irq);
#endif