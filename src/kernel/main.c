#include <kernel/kernel.h>
#include <kernel/time.h>
#include <kernel/multiboot2.h>
#include <kernel/mm.h>

void check_pgtable(addr_t);
void kernel_main()
{
    // halt();
    mboot_info_init();
    console_init();
    mboot_info_show();
    mm_init();
    // halt();
    // map_region(NULL, 0xFFFF804000000000, 0x4000000000, 512*PGSIZE, PTE_W, 0);
    // check_pgtable(0xFFFF804000000000);
    // klog("sizeof types:\n");
    // klog("int:%u\n",sizeof(int));
    // klog("long:%u\n",sizeof(long));
    // klog("long long:%u\n",sizeof(long long));
    // klog("u8:%u\n",sizeof(u8));
    // klog("u16:%u\n",sizeof(u16));
    // klog("u32:%u\n",sizeof(u32));
    // klog("u64:%u\n",sizeof(u64));
    // while(1);
    cprintf("Hello GeekToyOS!\n");
    // clock_init();
    
    halt();
    while(1);
}

void _klog(const char * file, int line, int level, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    if(level != LOG_INFO){
        kprintf("%s:%d:", file, line);
    }
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