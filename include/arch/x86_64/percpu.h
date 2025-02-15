#pragma once
#include <kernel/kernel.h>
#include <kernel/sched.h>

struct percpu {
    size_t  id;
    struct task* current_task;
    struct task *idle_task;
    struct runq *runqueue;
};