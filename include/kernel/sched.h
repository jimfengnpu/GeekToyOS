#ifndef SCHED_H
#define SCHED_H
#include <kernel/interrupt.h>

void sched_handler(trapframe_t *frame);
void sched_start();

#endif