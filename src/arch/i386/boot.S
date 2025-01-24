#define STACK_SIZE      0x4000
#define PAGE_SIZE       0x1000
#define MULTIBOOT2_BOOTLOADER_MAGIC             0x36d76289
#include <mem_layout.h>
/*  The flags for the Multiboot header. */
#define GRUB_MULTIBOOT_ARCHITECTURE_I386 (0)
#define MULTIBOOT2_HEADER_LEN (multiboot_header_end - multiboot_header)
#define MULTIBOOT2_HEADER_CHECKSUM \
    -(MULTIBOOT2_HEADER_MAGIC \
    + GRUB_MULTIBOOT_ARCHITECTURE_I386 \
    + MULTIBOOT2_HEADER_LEN)
#define MULTIBOOT2_HEADER_MAGIC                 0xe85250d6
#define MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS  3
# #define MULTIBOOT_HEADER_TAG_FRAMEBUFFER  5
#define MULTIBOOT_HEADER_TAG_END  0

.section  .multiboot
        .align 8
multiboot_header:
        .long   MULTIBOOT2_HEADER_MAGIC
        .long   GRUB_MULTIBOOT_ARCHITECTURE_I386
        .long   MULTIBOOT2_HEADER_LEN
        .long   MULTIBOOT2_HEADER_CHECKSUM
tag_entry:
        .align 8
        .short MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS
        .short 0
        .long 12
        .long start

tags_end:
        .align 8
        .short  MULTIBOOT_HEADER_TAG_END
        .short  0
        .long   8
multiboot_header_end:
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
