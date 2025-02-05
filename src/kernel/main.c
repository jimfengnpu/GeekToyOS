#include <kernel/kernel.h>
#include <kernel/multiboot2.h>
#include <kernel/mm.h>
#include <kernel/sched.h>
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
    sched_start();
    while(1)halt();

}

void _klog(const char * file, int line, int level, const char *fmt, ...)
{
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
}

void _panic(const char * file, int line, const char *fmt, ...)
{
    interrupt_disable();
    va_list ap;
    va_start(ap, fmt);
    kprintf("%s:%d:", file, line);
    vkprintf(fmt, ap);
    va_end(ap);
    halt();
}