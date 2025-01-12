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
        .section ".init.text"

        .globl  start
        .globl  ist_stack1, ist_stack2
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

        mov     $pud, %ebx
        mov     $pmd, %eax
        add     $(PAGE_SIZE), %eax
        or      $(PDE_ATTR_P | PDE_ATTR_RW), %eax
        mov     %eax, 8(%ebx)

        mov     $pmd, %eax
        add     $(2*PAGE_SIZE), %eax
        or      $(PDE_ATTR_P | PDE_ATTR_RW), %eax
        mov     %eax, 16(%ebx)

        mov     $pmd, %eax
        add     $(3*PAGE_SIZE), %eax
        or      $(PDE_ATTR_P | PDE_ATTR_RW), %eax
        mov     %eax, 24(%ebx)

        //mov     $pagetable, %eax
        //or      $(PDE_ATTR_P | PDE_ATTR_RW), %eax
        //mov     %eax, pmd

        xor     %ecx, %ecx
        //movl    $pagetable, %ebx
        movl    $pmd, %ebx
.loop_makepage:
        movl    %ecx, %eax
        sal     $21, %eax
        or      $(PDE_ATTR_P | PDE_ATTR_RW | PDE_ATTR_PS), %eax
        movl    %eax, (%ebx,%ecx,8)
        inc     %ecx
        cmp     $2048, %ecx
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
        ljmp    $0x08, $.long_mode

        # we shouldn't get here...
        
.loop:   hlt
        jmp     .loop
        .code64
.long_mode:
        mov     $(KERN_BASE), %rax
        addq  %rax, %rsp
        addq  %rax, %rbp
        call boot_main
        .align 0x1000
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
        .space  PAGE_SIZE * 4
//pagetable:
//        .space  PAGE_SIZE * 2
