#include <kernel/kernel.h>
#include <kernel/smp.h>
#include <kernel/clock.h>
#include <atomic.h>


static int smp_cpu_ready_count;

void smp_notify_ready(void)
{
    klog("SMP: cpu %d online\n", smp_cpuid());
    atomic_inc(&smp_cpu_ready_count);
}

void smp_init(void)
{
    if (smp_cpunum() == 1) {
        klog("SMP: no mp, fallback to single cpu\n");
        return;
    }
    smp_notify_ready();
    arch_smp_init();
    clock_sleep_watch_flag(1, &smp_cpu_ready_count, smp_cpunum(), 0);
    info("SMP: %d cpu online\n", smp_cpu_ready_count);
}
