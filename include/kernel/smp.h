#ifndef SMP_H
#define SMP_H
#include <kernel/kernel.h>
#include <kernel/sched.h>

// impl by arch
u8 smp_cpunum(void);
u8 smp_cpuid(void);
void arch_smp_init(void);


void smp_init(void);
void smp_notify_ready(void);

#endif