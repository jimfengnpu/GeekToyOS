#ifndef SCHED_H
#define SCHED_H
#include <kernel/interrupt.h>
#include <kernel/mm.h>


struct runq {

};

struct task {
    size_t pid;
    unsigned long task_flag;
    struct task_mm *mm;
    struct cpu_context* context;
};


void sched_handler(trapframe_t *frame);
void sched_start();

#endif