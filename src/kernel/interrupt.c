#include <kernel/interrupt.h>

isr_info interrupt_handlers[INTERRUPT_MAX];

// 注册一个中断处理函数
void register_interrupt_handler(u8 vec, u8 type, interrupt_handler_t h)
{
    interrupt_handlers[vec].type = type;
    interrupt_handlers[vec].handler = h;
}

// 调用中断处理函数
void interrupt_handler(trapframe_t *frame)
{
    struct isr_info* info = &interrupt_handlers[frame->int_no];
    if (info->type == ISR_IRQ) {
        interrupt_eoi(frame->int_no);
    }
    // cprintf("int: %d\n", frame->int_no);
    if (info->handler)
    {
        info->handler(frame);
    }
}