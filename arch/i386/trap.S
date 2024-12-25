    .section .text
.macro interrupt_errcode no
    .globl intr_\no
intr_\no:
    pushl   \no
    jmp    interrupt_common
.endm

.macro interrupt_no_errcode no
    .globl intr_\no
intr_\no:
    pushl   0
    pushl   \no
    jmp    interrupt_common
.endm

interrupt_no_errcode    0 // #DE
interrupt_no_errcode    1 // #DB
interrupt_no_errcode    2 // NMI
interrupt_no_errcode    3 // BREAK
interrupt_no_errcode    4 // #OF 溢出
interrupt_no_errcode    5 // #BR 越界
interrupt_no_errcode    6 // #UD 无效指令
interrupt_no_errcode    7 // #NM 数学协处理器不可用
interrupt_errcode       8 // #DF 双重故障
interrupt_no_errcode    9 // 协处理器跨段操作
interrupt_errcode       10// #TS 无效TSS
interrupt_errcode       11// #NP 段不存在
interrupt_errcode       12// #SS 栈错误
interrupt_errcode       13// #GP 常规保护
interrupt_errcode       14// #PF 页故障
interrupt_no_errcode    15// CPU 保留
interrupt_no_errcode    16// #MF 浮点处理单元错误
interrupt_errcode       17// #AC 对齐检查
interrupt_no_errcode    18// #MC 机器检查
interrupt_no_errcode    19// #XM SIMD(单指令多数据)浮点异常

// reserved
interrupt_no_errcode    20
interrupt_no_errcode    21
interrupt_no_errcode    22
interrupt_no_errcode    23
interrupt_no_errcode    24
interrupt_no_errcode    25
interrupt_no_errcode    26
interrupt_no_errcode    27
interrupt_no_errcode    28
interrupt_no_errcode    29
interrupt_no_errcode    30
interrupt_no_errcode    31

// custom
interrupt_no_errcode    128

    .globl interrupt_common
    .extern interrupt_handler
// 中断服务程序
interrupt_common:
	pusha                   // Pushes edi, esi, ebp, esp, ebx, edx, ecx, eax
	mov %ds, %ax
	pushl %eax                // 保存数据段描述符
	
	mov 0x10, %ax            // 加载内核数据段描述符表
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ss, %ax
	
	pushl %esp		// 此时的 esp 寄存器的值等价于 pt_regs 结构体的指针
	call interrupt_handler        // 在 C 语言代码里
	add $4, %esp 		// 清除压入的参数
	
	popl %ebx                 // 恢复原来的数据段描述符
	mov %bx, %ds
	mov %bx, %es
	mov %bx, %fs
	mov %bx, %gs
	mov %bx, %ss
	
	popa                     // Pops edi, esi, ebp, esp, ebx, edx, ecx, eax
	add 8, %esp               // 清理栈里的 error code 和 ISR
	iret

// 构造中断请求的宏
.macro irq no, vec
    .globl irq_\no
irq_\no:
    pushl $0
    pushl \vec
    jmp irq_common
.endm

irq   0,    32 	// 电脑系统计时器
irq   1,    33 	// 键盘
irq   2,    34 	// 与 IRQ9 相接，MPU-401 MD 使用
irq   3,    35 	// 串口设备
irq   4,    36 	// 串口设备
irq   5,    37 	// 建议声卡使用
irq   6,    38 	// 软驱传输控制使用
irq   7,    39 	// 打印机传输控制使用
irq   8,    40 	// 即时时钟
irq   9,    41 	// 与 IRQ2 相接，可设定给其他硬件
irq  10,    42 	// 建议网卡使用
irq  11,    43 	// 建议 AGP 显卡使用
irq  12,    44 	// 接 PS/2 鼠标，也可设定给其他硬件
irq  13,    45 	// 协处理器使用
irq  14,    46 	// IDE0 传输控制使用
irq  15,    47 	// IDE1 传输控制使用

.globl irq_common
.globl forkret_s
.extern irq_handler
irq_common:
	pusha                    // pushes edi, esi, ebp, esp, ebx, edx, ecx, eax
	
	mov %ds, %ax
	pushl %eax                 // 保存数据段描述符
	
	mov $0x10, %ax  		 // 加载内核数据段描述符
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss
	
	pushl %esp
	call irq_handler
	add $4, %esp

forkret_s:
	popl %ebx                   // 恢复原来的数据段描述符
	mov %bx, %ds
	mov %bx, %es
	mov %bx, %fs
	mov %bx, %gs
	mov %bx, %ss
	
	popa                     // Pops edi,esi,ebp...
	add $8, %esp     		 // 清理压栈的 错误代码 和 ISR 编号
	iret          		 // 出栈 CS, EIP, EFLAGS, SS, ESP
