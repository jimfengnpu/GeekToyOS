#pragma once
#include <kernel/kernel.h>
struct cpu_context
{
    u64 r15;
    u64 r14;
    u64 r13;
    u64 r12;
    u64 rbp;
    u64 rbx;
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
    u64 ip;
    u64 cs;
    u64 rflags;
    // x86_64无论是否发生了特权级切换CPU都会压栈
    u64 sp;
    u64 ss;
};