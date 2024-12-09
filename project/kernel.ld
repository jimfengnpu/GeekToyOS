#include <mem_layout.h>
ENTRY(start)
SECTIONS
{
        . = 0x100000;

        PROVIDE(kern_start = .);
        PROVIDE(kern_init_start = .);
        .init.text :
        {
                . = ALIGN(8);
                PROVIDE(multiboot_header_start = .);
                KEEP(*(.multiboot))
                PROVIDE(multiboot_header_end = .);
                *(.init.text)
                . = ALIGN(4096);
        }
        .init.data :
        {
                *(.init.data)
                . = ALIGN(4096);
        }

        PROVIDE(kern_init_end = .);

        . += KERN_BASE;

        PROVIDE(kern_text_start = . - KERN_BASE);

        .text : AT(ADDR(.text) - KERN_BASE)
        {
                *(.text)
                . = ALIGN(4096);
        }

        PROVIDE(kern_text_end = . - KERN_BASE);
        PROVIDE(kern_data_start = . - KERN_BASE);

        .data : AT(ADDR(.data) - KERN_BASE)
        {
                *(.data)
                *(.rodata)
                . = ALIGN(4096);
        }
        .bss : AT(ADDR(.bss) - KERN_BASE)
        {
                *(.bss)
                . = ALIGN(4096);
        }

        PROVIDE(kern_data_end = . - KERN_BASE);

        .stab : AT(ADDR(.stab) - KERN_BASE)
        {
                *(.stab)
                . = ALIGN(4096);
        }
        .stabstr : AT(ADDR(.stabstr) - KERN_BASE)
        {
                *(.stabstr)
                . = ALIGN(4096);
        }

        PROVIDE(kern_end = . - KERN_BASE);

        /DISCARD/ : { *(.comment) *(.eh_frame) }
}