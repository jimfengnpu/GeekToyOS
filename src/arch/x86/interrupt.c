#include <cpu.h>
#include <trap.h>
#include <protect.h>
#include <apic.h>
#include <interrupt.h>
#include <kernel/interrupt.h>

#define IO_PIC1   (0x20)	  // Master (IRQs 0-7)
#define IO_PIC2   (0xA0)	  // Slave  (IRQs 8-15)

#define IO_PIC1C  (IO_PIC1+1)
#define IO_PIC2C  (IO_PIC2+1)

static int apic_init_ok;

// apic.c
void lapic_eoi(u8 vec);
void lapic_enable();
// idt.c
void idt_load(void);

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
        
        // 暂时先关闭
        // 如果支持，使用APIC接管中断
        outb(IO_PIC1C, 0xFF);
        outb(IO_PIC2C, 0xFF);
}

// 重设 8259A 芯片
void pic_eoi(u32 intr_no)
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
		case IRQ_NMI:
			int_no = 2;
			// fallthrough
		default:
            // traceback((void *)frame->sp);
			panic(
				"%s:\n"
				"\tip: %p, sp: %p\n"
				"\tint_no: %u, err_code: %lu",
				intrname(int_no),
				(void *)frame->ip, (void *)frame->sp,
				int_no, (frame->err_code & 0xFFFFFFFF)
			);
	}
}


void exception_init(void)
{
	for (u8 i = 0; i < 32; i++) {
        register_interrupt_handler(i, ISR_EXCEPTION, exception_handler);
    }

	register_interrupt_handler(IRQ_NMI, ISR_EXCEPTION, exception_handler);
}

void interrupt_init(void)
{
    pic_init();
    if(apic_init()){
        apic_init_ok = 1;
    }
    idt_init();
    exception_init();
    interrupt_local_init();
}

void interrupt_local_init(void)
{
    idt_load();
    if(apic_init_ok){
        lapic_enable();
    }
}

void interrupt_eoi(u8 vec)
{
    if (apic_init_ok) {
        lapic_eoi(vec);
    } else {
        pic_eoi(vec);
    }
}

void interrupt_enable_irq(u8 irq)
{
    if (apic_init_ok) {
        ioapic_unmask(ioapic_isa_to_gsi(irq));
    }
}

void interrupt_disable_irq(u8 irq)
{
    if (apic_init_ok) {
        ioapic_mask(ioapic_isa_to_gsi(irq));
    }
}
