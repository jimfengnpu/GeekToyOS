#include <kernel/kernel.h>
#include <kernel/multiboot2.h>
#include <kernel/mm.h>
#include <paging.h>
#include <boot.h>
#include <gdt.h>
#include <interrupt.h>


extern pgd_t *kernel_pgd;

void clear_low_page()
{
    as_kaddr((pde_t*)pagedir)[0] = 0;
}

void boot_start()
{
    clear_low_page();
    gdt_init();
    kernel_main();
}

__init_text void boot_main(u32 magic, u32 mboot_info_phy)
{
    // init_paging();
    // enable_paging(CR0_PE|CR0_PG|CR0_WP);
    /* 此处开始调用函数不再使用init段*/
    mboot_info = (struct multiboot_tag*)kaddr(mboot_info_phy);
    kernel_pgd = kaddr(pagedir);
    boot_start();
}