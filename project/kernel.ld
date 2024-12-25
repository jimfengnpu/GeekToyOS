#include <mem_layout.h>
ENTRY(start)
SECTIONS
{
        . = 0x100000;

        PROVIDE(kern_start = .);
        PROVIDE(kern_text_start = .);
        .init.text :
        {
                . = ALIGN(8);
                PROVIDE(multiboot_header_start = .);
                KEEP(*(.multiboot))
                PROVIDE(multiboot_header_end = .);
                *(.init.text)
        }
        . += KERN_BASE;
        .text : AT(ADDR(.text) - KERN_BASE)
        {
                *(.text)
        }

        PROVIDE(kern_text_end = . - KERN_BASE);
        . -= KERN_BASE;

        PROVIDE(kern_data_start = .);
        .init.data :AT(ADDR(.init.data))
        {
                *(.init.data)
        }

        . += KERN_BASE;

        .data : AT(ADDR(.data) - KERN_BASE)
        {
                *(.data)
                *(.rodata)
        }
        . -= KERN_BASE;
        .init.bss :
        {
                *(.init.bss)
        }
        . += KERN_BASE;

        .bss : AT(ADDR(.bss) - KERN_BASE)
        {
                *(.bss)
        }

        PROVIDE(kern_data_end = . - KERN_BASE);

        .stab : AT(ADDR(.stab) - KERN_BASE)
        {
                *(.stab)
        }
        .stabstr : AT(ADDR(.stabstr) - KERN_BASE)
        {
                *(.stabstr)
        }

        PROVIDE(kern_end = . - KERN_BASE);

        /DISCARD/ : { *(.comment) *(.eh_frame) }
}