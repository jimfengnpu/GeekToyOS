#ifndef SECTION_H
#define SECTION_H

#define __section__(name) __attribute__((section(name)))

#define __init_text __section__(".init.text")
#define __init_data __section__(".init.data")

#define __mboot_header  __attribute__((section(".multiboot"),aligned(8)))

extern char kern_start[];
extern char multiboot_header_start[];
extern char multiboot_header_end[];
extern char kern_init_start[];
extern char kern_init_end[];
extern char kern_text_start[];
extern char kern_text_end[];
extern char kern_data_start[];
extern char kern_data_end[];
extern char kern_end[];

#endif