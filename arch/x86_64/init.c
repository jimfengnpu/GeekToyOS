#include <kernel/kernel.h>
#include <kernel/multiboot2.h>

#define MBOOT_HEADER_LENGTH 48

__mboot_header const struct multiboot_header header = {
    .magic = MULTIBOOT2_HEADER_MAGIC,
    .architecture = MULTIBOOT_ARCHITECTURE_I386,
    .header_length = MBOOT_HEADER_LENGTH,
    .checksum = -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 + MBOOT_HEADER_LENGTH)
};
__mboot_header const struct multiboot_header_tag_framebuffer fb_tag = {
    .type = MULTIBOOT_HEADER_TAG_FRAMEBUFFER,
    .flags = MULTIBOOT_HEADER_TAG_OPTIONAL,
    .size = 24,
    .width = 1080,
    .height = 768,
    .depth = 32
};
__mboot_header const struct multiboot_header_tag header_end = {
    .type = MULTIBOOT_HEADER_TAG_END,
    .size = 8
};


__init_text void boot_main()
{
    while(1);
}