#pragma once
#include <kernel/kernel.h>
struct cpu_context
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
    u32 ip;
    u16 cs;
    u16 padding2;
    u32 eflags;

    // 如果发生了特权级的切换CPU会压栈
    u32 sp;
    u16 ss;
    u16 padding3;
};