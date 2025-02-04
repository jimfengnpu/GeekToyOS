    .section .text
	.code64
.macro isr_err no
    .globl isr_\no
isr_\no:
    pushq   $\no
    jmp    interrupt_common
.endm

.macro isr_noerr no
    .globl isr_\no
isr_\no:
    pushq   $0
    pushq   $\no
    jmp    interrupt_common
.endm

.macro isr_irq no
    .globl isr_\no
isr_\no:
    pushq	$\no
    jmp interrupt_common
.endm

.altmacro
.macro batch_irq from, to
isr_irq		\from
.if \to-\from
batch_irq	%(\from + 1) ,\to
.endif
.endm

.altmacro
.macro batch_isr_func from, to
.quad   isr_\from
.if \to-\from
batch_isr_func	%(\from + 1) ,\to
.endif
.endm

isr_noerr    0 // #DE
isr_noerr    1 // #DB
isr_noerr    2 // NMI
isr_noerr    3 // BREAK
isr_noerr    4 // #OF 溢出
isr_noerr    5 // #BR 越界
isr_noerr    6 // #UD 无效指令
isr_noerr    7 // #NM 数学协处理器不可用
isr_err       8 // #DF 双重故障
isr_noerr    9 // 协处理器跨段操作
isr_err       10// #TS 无效TSS
isr_err       11// #NP 段不存在
isr_err       12// #SS 栈错误
isr_err       13// #GP 常规保护
isr_err       14// #PF 页故障
isr_noerr    15// CPU 保留
isr_noerr    16// #MF 浮点处理单元错误
isr_err       17// #AC 对齐检查
isr_noerr    18// #MC 机器检查
isr_noerr    19// #XM SIMD(单指令多数据)浮点异常

// reserved
isr_noerr    20
isr_noerr    21
isr_noerr    22
isr_noerr    23
isr_noerr    24
isr_noerr    25
isr_noerr    26
isr_noerr    27
isr_noerr    28
isr_noerr    29
isr_noerr    30
isr_noerr    31

// 构造中断请求
batch_irq 32, 127
batch_irq 128, 223
batch_irq 224, 255
	.globl isr_func
isr_func:
batch_isr_func 0, 100
batch_isr_func 101, 201
batch_isr_func 202, 255  
    .globl interrupt_common
    .extern interrupt_handler
// 中断服务程序
interrupt_common:
	pushq	%rdi
	pushq	%rsi
	pushq	%rdx
	pushq	%rcx
	pushq	%rax
	pushq	%r8
	pushq	%r9
	pushq	%r10
	pushq	%r11
	mov 	%rsp,	%rdi
	
	call interrupt_handler        // 在 C 语言代码里
	
	popq	%r11
	popq	%r10
	popq	%r9
	popq	%r8
	popq	%rax
	popq	%rcx
	popq	%rdx
	popq	%rsi
	popq	%rdi
	addq	$8, %rsp               // 清理栈里的 error code 和 ISR
	iretq
