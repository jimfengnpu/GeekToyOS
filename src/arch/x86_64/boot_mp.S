#define STACK_SIZE      0x4000
#define PAGE_SIZE       0x1000
#define MP_BASE         0x8000
#define LABEL(label) (MP_BASE + ((label) - mp_start))

/* CR0 */
#define CR0_PG (1 << 31)

/* CR4 */
#define CR4_PSE (1 << 4)
#define CR4_PAE (1 << 5)
#define CR4_PGE (1 << 7)

#define SELECTOR_INDEX (3)

        .code16
        .text
        .align 0x1000
        .extern pgd,gdt64_ptr32,gdt64_ptr64, mp_stacktop, smp_ap_started_flag
        .global mp_start, mp_end
mp_start:
        cli
        cld
        movl    $LABEL(_L_GDT_PTR), %eax
        lgdt    (%eax)
        movl    %cr0, %eax
        orl     $1, %eax
        movl    %eax, %cr0
        ljmp    $8, $LABEL(_L)
        .align 16
_L_GDT_table:
        .long 0
        .long 0
        .long 0x0000FFFF
        .long 0x00CF9A00    # flat code
        .long 0x0000FFFF
        .long 0x00CF9200    # flat data
_L_GDT_PTR:
        .short _L_GDT_PTR - _L_GDT_table - 1
        .long  LABEL(_L_GDT_table)
        .long 0, 0
        .align 64
        .code32
_L:
        movw    $16, %ax
        movw    %ax, %ds
        movw    %ax, %es
        movw    %ax, %fs
        movw    %ax, %gs
        movw    %ax, %ss

        # disable paging (UEFI may turn it on)
        mov     %cr0, %eax
        mov     $CR0_PG, %ebx
        not     %ebx
        and     %ebx, %eax
        mov     %eax, %cr0
    
        movl     $pgd, %eax
        movl     %eax, %cr3

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
        mov     $gdt64_ptr32, %eax
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
        ljmp  $(1 << SELECTOR_INDEX), $LABEL(.mp_long_mode)

        .code64
.mp_long_mode:
        movabs     $gdt64_ptr64, %rax
        lgdt    (%rax)
        movabs  mp_stacktop, %rax
        mov     %rax, %rsp
        mov     $boot_ap, %rax
        call    *%rax
mp_end: