#include <kernel/kernel.h>
#include <kernel/multiboot2.h>
#include <paging.h>
#include <boot.h>
#include <gdt.h>
#include <interrupt.h>

#define MBOOT_HEADER_LENGTH 24

__mboot_header const struct  multiboot_header  header = {
    .magic = MULTIBOOT2_HEADER_MAGIC,
    .architecture = MULTIBOOT_ARCHITECTURE_I386,
    .header_length = MBOOT_HEADER_LENGTH,
    .checksum = -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 + MBOOT_HEADER_LENGTH)
};
__mboot_header const struct  multiboot_header_tag header_end = {
    .type = MULTIBOOT_HEADER_TAG_END,
    .size = 8
};

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

__init_text void boot_main(u32 magic, u32 mboot_info_phy)
{
    init_paging();
    enable_paging(CR0_PE|CR0_PG|CR0_WP);
    /* 此处开始调用函数不再使用init段*/
    mboot_info = (struct multiboot_tag*)kaddr(mboot_info_phy);
    init_gdt();
    interrupt_init();
    kernel_main();

}