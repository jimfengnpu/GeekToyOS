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
/*  The flags for the Multiboot header. */
        .section ".init.text"

        .globl  start
start:
        .code32
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
        or      $(PDE_ATTR_P | PDE_ATTR_RW), %eax
        mov     %eax, pud

        mov     $pagetable, %eax
        or      $(PDE_ATTR_P | PDE_ATTR_RW), %eax
        mov     %eax, pmd

        xor     %ecx, %ecx
        movl    $pagetable, %ebx
.loop_makepage:
        movl    %ecx, %eax
        sal     $12, %eax
        or      $(PDE_ATTR_P | PDE_ATTR_RW ), %eax
        movl    %eax, (%ebx,%ecx,8)
        inc     %ecx
        cmp     $1024, %ecx
        jl      .loop_makepage

        # load page table
        mov     $pgd, %eax
        mov     %eax, %cr3

        mov     %cr4, %eax
        or      $(CR4_PAE), %eax
        mov     %eax, %cr4

        mov     $0xC0000080, %ecx
        rdmsr
        or      $(1 << 8), %eax
        wrmsr
        # enable paging
        mov     %cr0, %eax
        or      $CR0_PG, %eax
        mov     %eax, %cr0
        #jmp .loop
        # set up GDT
        mov     $gdt64_ptr, %eax
        lgdt    (%eax)
        #jmp     .loop
        # enter long mode by enabling EFER.LME
        # reload all the segment registers
        mov $(2 << SELECTOR_INDEX), %ax
        mov     %ax, %ds
        mov     %ax, %ss
        mov     %ax, %es
        mov     %ax, %fs
        mov     %ax, %gs

        
        # enter the 64-bit world within a long jmp
        #pushl   $(1 << SELECTOR_INDEX)
        #pushl   
        ljmp    $0x08, $boot_main

        # we shouldn't get here...
        
.loop:   hlt
        jmp     .loop

        .align 0x1000
        # global segment descriptor table
        .globl gdt64, gdt64_ptr
gdt64:
        .quad 0 # first one must be zero
gdt64_code_segment:
        .quad 0x002F9A000000FFFF # exec/read
gdt64_data_segment:
        .quad 0x000F92000000FFFF # read/write
gdt64_code_segment_compat:
        .quad 0x00CF9A000000FFFF # read/write
gdt64_ptr:
        .short gdt64_ptr - gdt64 - 1    # GDT limit
        .long gdt64                     # GDT Addr
        .section ".init.bss", "w", @nobits
        .align 0x1000
stack:
        .space  STACK_SIZE
stacktop:
        .align 0x1000
pgd:
        .space  PAGE_SIZE
pud:
        .space  PAGE_SIZE
pmd:
        .space  PAGE_SIZE
pagetable:
        .space  PAGE_SIZE * 2
