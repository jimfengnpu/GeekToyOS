#define STACK_SIZE      0x4000
#define PAGE_SIZE       0x1000

#define PDE_ATTR_P     (1 << 0)
#define PDE_ATTR_RW    (1 << 1)
#define PDE_ATTR_PS    (1 << 7)

/* CR0 */
#define CR0_PG (1 << 31)

/* CR4 */
#define CR4_PSE (1 << 4)
#define CR4_PAE (1 << 5)
#define CR4_PGE (1 << 7)

/* Segment selector */
#define SELECTOR_RPL (0)
#define SELECTOR_TI (2)
#define SELECTOR_INDEX (3)
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
#define MULTIBOOT_HEADER_TAG_FRAMEBUFFER  5
#define MULTIBOOT_HEADER_TAG_END  0

.section  .multiboot
.extern multiboot_header_start
.extern kern_start
.extern kern_data_end
.extern kern_end
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

tag_frame_buffer:
        .align 8
        .short MULTIBOOT_HEADER_TAG_FRAMEBUFFER
        .short 0
        .long 20
        .long 1920
        .long 1080
        .long 32

tags_end:
        .align 8
        .short  MULTIBOOT_HEADER_TAG_END
        .short  0
        .long   8
multiboot_header_end:
        .section ".init.text"

        .globl  start
        .globl  ist_stack1, ist_stack2, pgd, pud
        .code32
        .align 0x1000
start:
        cli
        # check for multiboot2 header
        cmp $MULTIBOOT2_BOOTLOADER_MAGIC, %eax
        jne .loop
        /*  Initialize the stack pointer. */
        movl    $(stacktop), %esp

        # set the boot information as parameters for boot_main()
        mov     %eax, %edi
        mov     %ebx, %esi

        movl    $0x80000001, %eax
        cpuid
        andl    $0x20000000, %edx
        cmp     $0, %edx
        je      .loop # no long mode support

        # clear eflags
        pushl   $0
        popf
        # disable paging (UEFI may turn it on)
        mov     %cr0, %eax
        mov     $CR0_PG, %ebx
        not     %ebx
        and     %ebx, %eax
        mov     %eax, %cr0
        

        # set up page table for booting stage
        # it's okay to write only 32bit here :)
        mov     $pud, %eax
        or      $(PDE_ATTR_P | PDE_ATTR_RW), %eax
        mov     %eax, pgd
        mov     $256, %ecx
        movl    $pgd, %ebx     
        mov     %eax, (%ebx, %ecx, 8)

        mov     $pmd, %eax
        # xor     %eax, %eax
        or      $(PDE_ATTR_P | PDE_ATTR_RW), %eax
        mov     %eax, pud

        
        /*mov     $pud, %ebx
        mov     $pmd, %eax
        add     $(PAGE_SIZE), %eax
        or      $(PDE_ATTR_P | PDE_ATTR_RW), %eax
        mov     %eax, 8(%ebx)*/
        /*
        mov     $pmd, %eax
        add     $(2*PAGE_SIZE), %eax
        or      $(PDE_ATTR_P | PDE_ATTR_RW), %eax
        mov     %eax, 16(%ebx)

        mov     $pmd, %eax
        add     $(3*PAGE_SIZE), %eax
        or      $(PDE_ATTR_P | PDE_ATTR_RW), %eax
        mov     %eax, 24(%ebx)
        */
        /*
        mov     $pagetable, %eax
        or      $(PDE_ATTR_P | PDE_ATTR_RW), %eax
        mov     %eax, pmd

        mov     $pmd, %ebx
        mov     $pagetable, %eax
        add     $(PAGE_SIZE), %eax
        or      $(PDE_ATTR_P | PDE_ATTR_RW), %eax
        mov     %eax, 8(%ebx)
        */
        xor     %ecx, %ecx
        //movl    $pagetable, %ebx
        movl    $pmd, %ebx
.loop_makepage:
        movl    %ecx, %eax
        sal     $21, %eax
        or      $(PDE_ATTR_P | PDE_ATTR_RW | PDE_ATTR_PS), %eax
        movl    %eax, (%ebx,%ecx,8)
        movl    $0, 4(%ebx,%ecx,8)
        inc     %ecx
        cmp     $512, %ecx
        jl      .loop_makepage
        
        xor     %eax, %eax
        mov     %eax, (pgd + 4)
        mov     %eax, (pud + 4)
        //mov     %eax, (pmd + 4)
        //mov     %eax, (pmd + 12)
        # load page table
        mov     $pgd, %eax
        mov     %eax, %cr3

        mov     %cr4, %eax
        or      $(CR4_PAE | CR4_PGE), %eax
        mov     %eax, %cr4

        mov     $0xC0000080, %ecx
        rdmsr
        or      $(1 << 8), %eax
        wrmsr
        # enable paging
        mov     %cr0, %eax
        or      $CR0_PG, %eax
        mov     %eax, %cr0
        
        # set up GDT
        mov     $gdt64_ptr, %eax
        lgdt    (%eax)

        # enter long mode by enabling EFER.LME
        # reload all the segment registers
        mov $(2 << SELECTOR_INDEX), %ax
        mov     %ax, %ds
        mov     %ax, %ss
        mov     %ax, %es
        mov     %ax, %fs
        mov     %ax, %gs

        
        # enter the 64-bit world within a long jmp 
        ljmp  $(1 << SELECTOR_INDEX), $.long_mode

        # we shouldn't get here...
        
.loop:   hlt
        jmp     .loop
        .code64
.long_mode:
        mov     $(KERN_BASE), %rax
        addq  %rax, %rsp
        jmp boot_main
        .align 0x10
        # global segment descriptor table
        .globl gdt64, gdt64_ptr
gdt64:
        .quad 0 # first one must be zero
gdt64_kcode:
        .quad 0x00209A0000000000 # exec/read
gdt64_kdata:
        .quad 0x0000920000000000 # read/write
gdt64_ucode32:
        .quad 0x0000000000000000 # read/write
gdt64_ucode64:
        .quad 0x0020FA0000000000
gdt64_udata:
        .quad 0x0000F20000000000
gdt64_tss:
        .short 103
        .short 0
        .long 0x00008900
        .quad 0
gdt64_ptr:
        .short gdt64_ptr - gdt64 - 1    # GDT limit
        .long gdt64                     # GDT Addr
        .section ".init.bss", "w", @nobits
        .align 0x1000
stack:
        .space  STACK_SIZE
stacktop:
        .space  PAGE_SIZE
ist_stack1:
        .space  PAGE_SIZE
ist_stack2:
        .align 0x1000
pgd:
        .space  PAGE_SIZE
pud:
        .space  PAGE_SIZE
pmd:
        .space  PAGE_SIZE
//pagetable:
//        .space  PAGE_SIZE
