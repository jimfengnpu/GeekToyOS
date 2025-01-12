#ifndef MULTIBOOT2_H
#define MULTIBOOT2_H
#include <kernel/kernel.h>
#include <kernel/multiboot2_s.h>

extern struct multiboot_tag* mboot_info;
void mboot_info_list();
struct multiboot_tag * mboot_get_mboot_info(u32 type);
#endif