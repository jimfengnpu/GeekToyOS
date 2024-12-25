#include <cpu.h>
#include <trap.h>
#include <interrupt.h>

#define IO_PIC1   (0x20)	  // Master (IRQs 0-7)
#define IO_PIC2   (0xA0)	  // Slave  (IRQs 8-15)

#define IO_PIC1C  (IO_PIC1+1)
#define IO_PIC2C  (IO_PIC2+1)

// 设置 8259A 芯片
void init_interrupt(void)
{
        // 重新映射 IRQ 表
        // 两片级联的 Intel 8259A 芯片
        // 主片端口 0x20 0x21
        // 从片端口 0xA0 0xA1
        
        // 初始化主片、从片
        // 0001 0001
        outb(IO_PIC1, 0x11);
        outb(IO_PIC2, 0x11);

        // 设置主片 IRQ 从 0x20(32) 号中断开始
        outb(IO_PIC1C, 0x20);

        // 设置从片 IRQ 从 0x28(40) 号中断开始
        outb(IO_PIC2C, 0x28);
        
        // 设置主片 IR2 引脚连接从片
        outb(IO_PIC1C, 0x04);

        // 告诉从片输出引脚和主片 IR2 号相连
        outb(IO_PIC2C, 0x02);
        
        // 设置主片和从片按照 8086 的方式工作
        outb(IO_PIC1C, 0x01);
        outb(IO_PIC2C, 0x01);
        
        // 设置主从片允许中断
        outb(IO_PIC1C, 0x0);
        outb(IO_PIC2C, 0x0);
}

// 重设 8259A 芯片
void clear_interrupt(u32 intr_no)
{
        // 发送中断结束信号给 PICs
        // 按照我们的设置，从 32 号中断起为用户自定义中断
        // 因为单片的 Intel 8259A 芯片只能处理 8 级中断
        // 故大于等于 40 的中断号是由从片处理的
        if (intr_no >= 40) {
                // 发送重设信号给从片
                outb(IO_PIC2, 0x20);
        }
        // 发送重设信号给主片
        outb(IO_PIC1, 0x20);
}

#define INTERRUPT_MAX 256

// 中断描述符
typedef
struct idt_entry_t {
        u16 base_lo;        // 中断处理函数地址 15～0 位
        u16 sel;            // 目标代码段描述符选择子
        u8  always0;        // 置 0 段
        u8  flags;          // 一些标志，文档有解释
        u16 base_hi;        // 中断处理函数地址 31～16 位
}__attribute__((packed)) idt_entry_t;

// IDTR
typedef
struct idt_ptr_t {
        u16 limit;        // 限长
        u32 base;         // 基址
} __attribute__((packed)) idt_ptr_t;

// 中断描述符表
static idt_entry_t idt_entries[INTERRUPT_MAX] __attribute__((aligned(16)));

// IDTR
static idt_ptr_t idt_ptr;

// 中断处理函数的指针数组
static interrupt_handler_t interrupt_handlers[INTERRUPT_MAX] __attribute__((aligned(4)));

// 设置中断描述符
static void idt_set_gate(u8 num, u32 base, u16 sel, u8 flags);

// 声明加载 IDTR 的函数
extern void idt_flush(u32);

// 中断处理函数指针类型
typedef void (*intr_irq_func_t)();

// 中断处理函数指针数组
static intr_irq_func_t intr_irq_func[INTERRUPT_MAX] = {
    [0] = &intr_0,
    [1] = &intr_1,
    [2] = &intr_2,
    [3] = &intr_3,
    [4] = &intr_4,
    [5] = &intr_5,
    [6] = &intr_6,
    [7] = &intr_7,
    [8] = &intr_8,
    [9] = &intr_9,
    [10] = &intr_10,
    [11] = &intr_11,
    [12] = &intr_12,
    [13] = &intr_13,
    [14] = &intr_14,
    [15] = &intr_15,
    [16] = &intr_16,
    [17] = &intr_17,
    [18] = &intr_18,
    [19] = &intr_19,
    [20] = &intr_20,
    [21] = &intr_21,
    [22] = &intr_22,
    [23] = &intr_23,
    [24] = &intr_24,
    [25] = &intr_25,
    [26] = &intr_26,
    [27] = &intr_27,
    [28] = &intr_28,
    [29] = &intr_29,
    [30] = &intr_30,
    [31] = &intr_31,

    [32] = &irq_0,
    [33] = &irq_1,
    [34] = &irq_2,
    [35] = &irq_3,
    [36] = &irq_4,
    [37] = &irq_5,
    [38] = &irq_6,
    [39] = &irq_7,
    [40] = &irq_8,
    [41] = &irq_9,
    [42] = &irq_10,
    [43] = &irq_11,
    [44] = &irq_12,
    [45] = &irq_13,
    [46] = &irq_14,
    [47] = &irq_15,
};

// 初始化中断描述符表
void init_idt(void)
{
    init_interrupt();

    idt_ptr.limit = sizeof(idt_entry_t) * INTERRUPT_MAX - 1;
    idt_ptr.base = (u32)&idt_entries;

    // 0~31:  用于 CPU 的中断处理
    // 32~47: Intel 保留
    for (u32 i = 0; i < 48; ++i)
    {
        idt_set_gate(i, (u32)intr_irq_func[i], 0x08, 0x8E);
    }

    // 128 (0x80) 将来用于实现系统调用
    idt_set_gate(128, (u32)intr_128, 0x08, 0xEF);

    // 更新设置中断描述符表
    asm volatile(
        "lidt (%0)"::"r"(&idt_ptr)
    );
}

// 设置中断描述符
static void idt_set_gate(u8 num, u32 base, u16 sel, u8 flags)
{
    idt_entries[num].base_lo = base & 0xFFFF;
    idt_entries[num].base_hi = (base >> 16) & 0xFFFF;

    idt_entries[num].sel = sel;
    idt_entries[num].always0 = 0;

    idt_entries[num].flags = flags;
}

static const char *intrname(u32 intrno)
{
    static const char *const intrnames[] = {
        "Divide error",
        "Debug",
        "Non-Maskable Interrupt",
        "Breakpoint",
        "Overflow",
        "BOUND Range Exceeded",
        "Invalid Opcode",
        "Device Not Available",
        "Double Fault",
        "Coprocessor Segment Overrun",
        "Invalid TSS",
        "Segment Not Present",
        "Stack Fault",
        "General Protection",
        "Page Fault",
        "(unknown trap)",
        "x87 FPU Floating-Point Error",
        "Alignment Check",
        "Machine-Check",
        "SIMD Floating-Point Exception"};

    if (intrno < sizeof(intrnames) / sizeof(const char *const))
    {
        return intrnames[intrno];
    }

    return "(unknown trap)";
}

// 调用中断处理函数
void interrupt_handler(pt_regs_t *regs)
{
    if (interrupt_handlers[regs->int_no])
    {
        interrupt_handlers[regs->int_no](regs);
    }
    else
    {
        halt();  
    }
}

// 注册一个中断处理函数
void register_interrupt_handler(u8 n, interrupt_handler_t h)
{
    interrupt_handlers[n] = h;
}

// IRQ 处理函数
void irq_handler(pt_regs_t *regs)
{
    clear_interrupt(regs->int_no);

    if (interrupt_handlers[regs->int_no])
    {
        interrupt_handlers[regs->int_no](regs);
    }
}
