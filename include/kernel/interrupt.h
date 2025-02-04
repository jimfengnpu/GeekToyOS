#ifndef INTERRUPT_H
#define INTERRUPT_H
#include <interrupt.h>
#include <kernel/kernel.h>

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

extern isr_info interrupt_handlers[INTERRUPT_MAX];

#define CLOCK_IRQ	0


void register_interrupt_handler(u8 vec, u8 type, interrupt_handler_t h);

#endif