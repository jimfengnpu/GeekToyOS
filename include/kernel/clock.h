#ifndef CLOCK_H
#define CLOCK_H
#include <kernel/kernel.h>
#include <kernel/interrupt.h>
#define HZ             100
typedef unsigned long clock_t;
extern u32  current_timestamp;
extern clock_t ticks;

void clock_handler(trapframe_t *frame);
void clock_sleep(clock_t sleep_tick);
u32 arch_get_hw_timestamp();
int arch_clock_init();
void arch_clock_start();
int arch_local_clock_enable();
#endif