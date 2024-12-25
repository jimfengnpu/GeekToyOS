#ifndef X86_INTERRUPT_H
#define X86_INTERRUPT_H
#include <kernel/types.h>
// 中断保存的寄存器类型
typedef struct pt_regs_t
{

    // 用于保存用户的数据段描述符
    u16 ds;
    u16 padding1;

    // 从 edi 到 eax 由 pusha 指令压栈
    u32 edi;
    u32 esi;
    u32 ebp;
    u32 oesp;
    u32 ebx;
    u32 edx;
    u32 ecx;
    u32 eax;

    // 中断号(内核代码自行压栈)
    u32 int_no;

    // 错误代码(有中断错误代码的中断会由CPU压栈)
    u32 err_code;

    // 以下由处理器自动压栈
    u32 eip;
    u16 cs;
    u16 padding2;
    u32 eflags;

    // 如果发生了特权级的切换CPU会压栈
    u32 esp;
    u16 ss;
    u16 padding3;
} pt_regs_t;

// 定义中断处理函数指针
typedef void (*interrupt_handler_t)(pt_regs_t *);

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

// 定义IRQ
#define IRQ0 32  // 电脑系统计时器
#define IRQ1 33  // 键盘
#define IRQ2 34  // 与 IRQ9 相接，MPU-401 MD 使用
#define IRQ3 35  // 串口设备
#define IRQ4 36  // 串口设备
#define IRQ5 37  // 建议声卡使用
#define IRQ6 38  // 软驱传输控制使用
#define IRQ7 39  // 打印机传输控制使用
#define IRQ8 40  // 即时时钟
#define IRQ9 41  // 与 IRQ2 相接，可设定给其他硬件
#define IRQ10 42 // 建议网卡使用
#define IRQ11 43 // 建议 AGP 显卡使用
#define IRQ12 44 // 接 PS/2 鼠标，也可设定给其他硬件
#define IRQ13 45 // 协处理器使用
#define IRQ14 46 // IDE0 传输控制使用
#define IRQ15 47 // IDE1 传输控制使用

// 初始化中断描述符表
void init_idt(void);


// 注册一个中断处理函数
void register_interrupt_handler(u8 n, interrupt_handler_t h);
#endif