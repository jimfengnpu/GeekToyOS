#include <kernel/kernel.h>
#include <kernel/multiboot2.h>
#include <kernel/mm.h>
#include <kernel/clock.h>
#include <kernel/sched.h>
#include <kernel/smp.h>
#include <kernel/spinlock.h>
#include <drivers/acpi.h>

void kernel_main()
{
    mboot_info_init();
    mem_init();
    console_init();
    mboot_info_show();

    mm_init();
    acpi_init();
    interrupt_init();
    clock_init();
    interrupt_enable();
    
    cprintf("Hello GeekToyOS!\n");
    
    smp_init();
    sched_start();
}

void ap_main()
{
    interrupt_local_init();
    interrupt_enable();
    smp_percpu_init();
    smp_notify_ready();
    sched_start();
}

static struct spinlock klog_lock;

void _klog(const char * file, int line, int level, const char *fmt, ...)
{
    acquire(&klog_lock);
    va_list ap;
    va_start(ap, fmt);
    if(level != LOG_INFO){
        kprintf("%s:%d:", file, line);
    }
    if(level == LOG_INFO) {
        vcprintf(fmt, ap);
    }else {
        vkprintf(fmt, ap);
    }
    va_end(ap);
    release(&klog_lock);
}

void _panic(const char * file, int line, const char *fmt, ...)
{
    acquire(&klog_lock); // 使得其他cpu不再能输出
    interrupt_disable();
    va_list ap;
    va_start(ap, fmt);
    cprintf("%s:%d:", file, line);
    vcprintf(fmt, ap);
    va_end(ap);
    halt();
}

void traceback(void** sp)
{
    cprintf("traceback:\n");
    int i = 0;
    while(sp && i < 6 ){
        cprintf("\t->%lx\n", *(sp+1));
        sp = *sp;
    }
}