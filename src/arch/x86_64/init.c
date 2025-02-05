#include <kernel/kernel.h>
#include <kernel/multiboot2.h>
#include <mm.h>
#include <interrupt.h>


extern char pgd[];
extern pgd_t *kernel_pgd; // in x86_64/mm.c


void boot_start()
{
    fetch_cpu_feature();
    // clear low page
    kernel_pgd[0] = 0;
    kernel_main();
}

__init_text void boot_main(u32 magic, u32 mboot_info_phy)
{
    mboot_info = (struct multiboot_tag*)kaddr(mboot_info_phy);
    kernel_pgd = (pgd_t*)kaddr(pgd);
    boot_start();
}