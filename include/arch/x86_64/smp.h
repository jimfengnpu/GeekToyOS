#pragma once
#include <kernel/kernel.h>
u8 smp_cpunum(void);
u8 smp_cpuid(void);
void smp_init(void);