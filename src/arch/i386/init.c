#include <kernel/kernel.h>
#include <kernel/multiboot2.h>
#include <kernel/mm.h>
#include <paging.h>
#include <boot.h>
#include <gdt.h>
#include <interrupt.h>


__init_text void init_paging()
{
    int offset = PDX(KERN_BASE);
    pagedir[0] = (pde_t)(pagetable) | (PTE_P); // 必需，启用页表后的下一条指令还在使用低地址
    for(int i = offset, index = 0; i < NPDENTRIES; i++, index++) // [0xC0000000 -- 0xFFFFFFFF] PDE
    {
        pagedir[i] = (pde_t)(pagetable + index) | (PTE_P|PTE_W); 
    }
    for(addr_t va = KERN_BASE; va < KMAPPING_BASE; va += PGSIZE) // [0xC0000000 -- 0xF0000000) PTE
    {
        pagetable[PDX(va) - offset][PTX(va)] = paddr(va) | (PTE_P|PTE_W);
    }
}

extern pgd_t *kernel_pgd;

void clear_low_page()
{
    as_kaddr((pde_t*)pagedir)[0] = 0;
}

void boot_start()
{
    clear_low_page();
    gdt_init();
    mm_init();
    interrupt_init();
    kernel_main();
}

__init_text void boot_main(u32 magic, u32 mboot_info_phy)
{
    init_paging();
    enable_paging(CR0_PE|CR0_PG|CR0_WP);
    /* 此处开始调用函数不再使用init段*/
    mboot_info = (struct multiboot_tag*)kaddr(mboot_info_phy);
    kernel_pgd = kaddr(pagedir);
    boot_start();
}