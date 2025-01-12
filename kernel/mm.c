#include <kernel/kernel.h>
#include <kernel/mm.h>
#include <kernel/multiboot2.h>

void mm_init()
{
    struct multiboot_tag *tag = mboot_get_mboot_info(MULTIBOOT_TAG_TYPE_MMAP);
    
}