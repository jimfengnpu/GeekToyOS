#ifndef INTERRUPT_H
#define INTERRUPT_H
#include <interrupt.h>
#include <kernel/kernel.h>

typedef struct{
    void (*init)();
    void (*enable_irq)(u8);
    void (*disable_irq)(u8);
} interrupt_pic_t;

// 定义中断处理函数指针
typedef void (*interrupt_handler_t)(trapframe_t *);

enum isr_type {
	ISR_IRQ, // Normal IRQs that require EOI
	ISR_EXCEPTION, // CPU exceptions like page faults
	ISR_NOP, // NOP for spurious interrupts
};

typedef struct isr_info{
    u8  type;
    interrupt_handler_t handler;
}isr_info;

extern interrupt_pic_t *interrupt_opts;
#define _chk_val_opts(ptr) if((ptr))
#define enable_irq(irq) _chk_val_opts(interrupt_opts) interrupt_opts->enable_irq((irq))
#define disable_irq(irq) _chk_val_opts(interrupt_opts) interrupt_opts->disable_irq((irq))

#endif