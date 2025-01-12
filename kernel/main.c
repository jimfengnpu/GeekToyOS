#include <kernel/kernel.h>
#include <kernel/time.h>
#include <kernel/multiboot2.h>

void kernel_main()
{
    console_init();
    klog("sizeof types:\n");
    klog("int:%u\n",sizeof(int));
    klog("long:%u\n",sizeof(long));
    klog("long long:%u\n",sizeof(long long));
    klog("u8:%u\n",sizeof(u8));
    klog("u16:%u\n",sizeof(u16));
    klog("u32:%u\n",sizeof(u32));
    klog("u64:%u\n",sizeof(u64));
    // while(1);
    mboot_info_list();
    cprintf("Hello GeekToyOS!\n");
    // clock_init();
    
    
    while(1);
}

void _klog(const char * file, int line, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    kprintf("%s:%d:", file, line);
    vkprintf(fmt, ap);
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