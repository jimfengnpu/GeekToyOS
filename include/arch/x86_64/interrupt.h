#ifndef X86_INTERRUPT_H
#define X86_INTERRUPT_H
#include <lib/types.h>
// 中断保存的寄存器类型
typedef struct Trapframe
{
    u64 r11;
    u64 r10;
    u64 r9;
    u64 r8;
    u64 rax;
    u64 rcx;
    u64 rdx;
    u64 rsi;
    u64 rdi;
    u64 int_no;
    u64 err_code;
    u64 rip;
    u64 cs;
    u64 rflags;
    // x86_64无论是否发生了特权级切换CPU都会压栈
    u64 rsp;
    u64 ss;
} trapframe_t;


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
void interrupt_init(void);

#endif