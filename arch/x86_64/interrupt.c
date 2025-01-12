#include <cpu.h>
#include <trap.h>
#include <protect.h>
#include <interrupt.h>
#include <kernel/interrupt.h>

#define IO_PIC1   (0x20)	  // Master (IRQs 0-7)
#define IO_PIC2   (0xA0)	  // Slave  (IRQs 8-15)

#define IO_PIC1C  (IO_PIC1+1)
#define IO_PIC2C  (IO_PIC2+1)

void pic_init(void)
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

        // x86_64 使用APIC接管中断
        outb(IO_PIC1C, 0xFF);
        outb(IO_PIC2C, 0xFF);
        interrupt_enable();
}

#define INTERRUPT_MAX 256

// 中断描述符
typedef
struct idt_entry_t {
        u16 base_lo;        // 中断处理函数地址 15～0 位
        u16 sel;            // 目标代码段描述符选择子
        u8  ist;            // legacy 置 0 段, used for x86_64 TSS
        u8  flags;          // 一些标志，文档有解释
        u16 base_mid;        // 中断处理函数地址 31～16 位
        u32 bash_hi;
        u32 __zero;
}__attribute__((packed)) idt_entry_t;

// IDTR
typedef
struct idt_ptr_t {
        u16 limit;        // 限长
        addr_t base;         // 基址
} __attribute__((packed)) idt_ptr_t;


// 中断描述符表
static idt_entry_t idt_entries[INTERRUPT_MAX] __attribute__((aligned(16)));

// IDTR
static idt_ptr_t idt_ptr;

// 中断处理函数的指针数组
static isr_info interrupt_handlers[INTERRUPT_MAX];

// 设置中断描述符
static void idt_set_gate(u8 num, addr_t base, u8 flags);

// 声明加载 IDTR 的函数
extern void idt_flush(u32);

// 中断处理函数指针类型
typedef void (*intr_func_t)();

// 中断处理函数指针数组
extern intr_func_t isr_func[INTERRUPT_MAX];
// 设置中断描述符
static void idt_set_gate(u8 num, addr_t base, u8 flags)
{
    idt_entries[num].base_lo = base & 0xFFFF;
    idt_entries[num].base_mid = (base >> 16) & 0xFFFF;
    idt_entries[num].bash_hi = (base >> 32) & 0xFFFFFFFF;

    idt_entries[num].sel = GD_SEL(SEG_KTEXT);
    idt_entries[num].ist = IST_NONE;

    idt_entries[num].flags = flags;
}

// 初始化中断描述符表
void init_idt(void)
{

    idt_ptr.limit = sizeof(idt_entry_t) * INTERRUPT_MAX - 1;
    idt_ptr.base = (addr_t)&idt_entries;

    u32 vec = 0;
    for(;vec < 256; vec++)
    {
        idt_set_gate(vec, (addr_t)isr_func[vec], DA_386IGate);
    }
    idt_entries[INT_DOUBLE_FAULT].ist = IST_DOUBLE_FAULT;
    idt_entries[INT_NMI].ist = IST_NMI;

}


static const char *intrname(u32 intrno)
{
    const char *const intrnames[] = {
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

static void exception_handler(trapframe_t *frame)
{
	u8 int_no = (u8)(frame->int_no);
	switch (int_no) {
		case INT_PAGE_FAULT:
			__builtin_unreachable();
			break;
		// case IRQ_NMI:
		// 	int_no = 2;
			// fallthrough
		default:
			panic(
				"%s:\n"
				"\trip: %p, rsp: %p\n"
				"\tint_no: %u, err_code: %lu",
				intrname(int_no),
				(void *)frame->rip, (void *)frame->rsp,
				int_no, (frame->err_code & 0xFFFFFFFF)
			);
	}
}
// 注册一个中断处理函数
void register_interrupt_handler(u8 vec, u8 type, interrupt_handler_t h)
{
    interrupt_handlers[vec].type = type;
    interrupt_handlers[vec].handler = h;
}

void init_exceptions(void)
{
	for (u8 i = 0; i < 32; i++) {
        register_interrupt_handler(i, ISR_EXCEPTION, exception_handler);
    }

	// register_interrupt_handler(IRQ_NMI, ISR_EXCEPTION, exception_handler);
}

void interrupt_init(void)
{
    pic_init();
    init_idt();
    init_exceptions();
    // 更新设置中断描述符表
    asm volatile(
        "lidt (%0)"::"r"(&idt_ptr)
    );

}
// 调用中断处理函数
void interrupt_handler(trapframe_t *frame)
{
    struct isr_info* info = &interrupt_handlers[frame->int_no];
    if (info->handler)
    {
        info->handler(frame);
    }
    
}

static void pic_disable_irq()
{

}
