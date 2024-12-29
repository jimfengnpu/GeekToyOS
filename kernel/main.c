#include <kernel/console.h>

void kernel_main()
{
    console_init();
    cprintf("Hello GeekToyOS!");
    while(1);
}