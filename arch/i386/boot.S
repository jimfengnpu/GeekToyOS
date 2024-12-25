#define STACK_SIZE      0x4000
#define PAGE_SIZE       0x1000
#define MULTIBOOT2_BOOTLOADER_MAGIC             0x36d76289
#include <mem_layout.h>
/*  The flags for the Multiboot header. */
        .section ".init.text"

        .globl  start
        ; .globl  multiboot_header
        .globl  pagedir
        .globl  pagetable
        .globl  enable_paging
; start:
start:
        # check for multiboot2 header
        cmp $MULTIBOOT2_BOOTLOADER_MAGIC, %eax
        jne .loop
        /*  Initialize the stack pointer. */
        movl    $(stacktop), %esp

        /*  Reset EFLAGS. */
        pushl   $0
        popf

        /*  Push the pointer to the Multiboot information structure. */
        pushl   %ebx
        /*  Push the magic value. */
        pushl   %eax

        /*  Now enter the C main function... */
        call    boot_main
        
.loop:   hlt
        jmp     .loop
enable_paging: /* enable_paging(flag)*/
        pushl   %ebp
        movl    %esp, %ebp
        leal    pagedir, %eax
        movl    %eax, %cr3 
        movl    %cr0, %eax
        orl     8(%ebp), %eax
        movl    %eax, %cr0
        addl    $(KERN_BASE), %esp
        popl    %ebp
        addl    $(KERN_BASE), %ebp
        ret
        .section ".init.bss",  "w", @nobits
stack:
        .space  STACK_SIZE
stacktop:
        .align  PAGE_SIZE
pagedir:
        .space  PAGE_SIZE
pagetable:
        .space  PAGE_SIZE*256
