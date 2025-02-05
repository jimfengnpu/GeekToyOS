#define STACK_SIZE      0x4000
#define PAGE_SIZE       0x1000

#define PDE_ATTR_P     (1 << 0)
#define PDE_ATTR_RW    (1 << 1)
/* CR0 */
#define CR0_PG (1 << 31)
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
#define MULTIBOOT_HEADER_TAG_ADDRESS    2
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
tag_addr:
        .align 8
        .short MULTIBOOT_HEADER_TAG_ADDRESS
        .short 0
        .long  24
        .long  multiboot_header_start
        .long  kern_start
        .long  kern_data_end
        .long  kern_end   
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
        cli
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

        /*  make kernel pagetable */
        mov     $pagetable, %eax
        or      $(PDE_ATTR_P | PDE_ATTR_RW), %eax
        mov     %eax, pagedir
        mov     $768, %ecx
        movl    $pagedir, %ebx     
        mov     %eax, (%ebx, %ecx, 4)

        xor     %ecx, %ecx
        movl    $pagetable, %ebx
.loop_makepage:
        movl    %ecx, %eax
        sal     $12, %eax
        or      $(PDE_ATTR_P | PDE_ATTR_RW), %eax
        movl    %eax, (%ebx,%ecx,4)
        inc     %ecx
        cmp     $1024, %ecx
        jl      .loop_makepage

        leal    pagedir, %eax
        movl    %eax, %cr3 
        movl    %cr0, %eax
        orl     $CR0_PG, %eax
        movl    %eax, %cr0
        addl    $(KERN_BASE), %esp
        /*  Now enter the C main function... */
        call    boot_main
        
.loop:   hlt
        jmp     .loop
enable_paging: /* enable_paging(flag)*/
        pushl   %ebp
        movl    %esp, %ebp
        
        ret
        .section ".init.bss",  "w", @nobits
stack:
        .space  STACK_SIZE
stacktop:
        .align  PAGE_SIZE
pagedir:
        .space  PAGE_SIZE
pagetable:
        .space  PAGE_SIZE
