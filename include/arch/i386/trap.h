#ifndef X86_TRAP_H
#define X86_TRAP_H
#include <interrupt.h>

// 调用中断处理函数
void interrupt_handler(pt_regs_t *regs);

// 声明中断处理函数 0-19 属于 CPU 的异常中断
// ISR:中断服务程序(interrupt service routine)
void intr_0();  // 0 #DE 除 0 异常
void intr_1();  // 1 #DB 调试异常
void intr_2();  // 2 NMI
void intr_3();  // 3 BP 断点异常
void intr_4();  // 4 #OF 溢出
void intr_5();  // 5 #BR 对数组的引用超出边界
void intr_6();  // 6 #UD 无效或未定义的操作码
void intr_7();  // 7 #NM 设备不可用(无数学协处理器)
void intr_8();  // 8 #DF 双重故障(有错误代码)
void intr_9();  // 9 协处理器跨段操作
void intr_10(); // 10 #TS 无效TSS(有错误代码)
void intr_11(); // 11 #NP 段不存在(有错误代码)
void intr_12(); // 12 #SS 栈错误(有错误代码)
void intr_13(); // 13 #GP 常规保护(有错误代码)
void intr_14(); // 14 #PF 页故障(有错误代码)
void intr_15(); // 15 CPU 保留
void intr_16(); // 16 #MF 浮点处理单元错误
void intr_17(); // 17 #AC 对齐检查
void intr_18(); // 18 #MC 机器检查
void intr_19(); // 19 #XM SIMD(单指令多数据)浮点异常

// 20-31 Intel 保留
void intr_20();
void intr_21();
void intr_22();
void intr_23();
void intr_24();
void intr_25();
void intr_26();
void intr_27();
void intr_28();
void intr_29();
void intr_30();
void intr_31();

// 32～255 用户自定义异常
void intr_128();

// IRQ 处理函数
void irq_handler(pt_regs_t *regs);



// 声明 IRQ 函数
// IRQ:中断请求(Interrupt Request)
void irq_0();  // 电脑系统计时器
void irq_1();  // 键盘
void irq_2();  // 与 IRQ9 相接，MPU-401 MD 使用
void irq_3();  // 串口设备
void irq_4();  // 串口设备
void irq_5();  // 建议声卡使用
void irq_6();  // 软驱传输控制使用
void irq_7();  // 打印机传输控制使用
void irq_8();  // 即时时钟
void irq_9();  // 与 IRQ2 相接，可设定给其他硬件
void irq_10(); // 建议网卡使用
void irq_11(); // 建议 AGP 显卡使用
void irq_12(); // 接 PS/2 鼠标，也可设定给其他硬件
void irq_13(); // 协处理器使用
void irq_14(); // IDE0 传输控制使用
void irq_15(); // IDE1 传输控制使用

#endif