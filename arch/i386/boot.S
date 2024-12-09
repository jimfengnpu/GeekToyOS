#define STACK_SIZE      0x4000


/*  The flags for the Multiboot header. */
        .section ".init.text"

        .globl  start
        ; .globl  multiboot_header
; start:
start:
        /*  Initialize the stack pointer. */
        movl    $(stack + STACK_SIZE), %esp

        /*  Reset EFLAGS. */
        pushl   $0
        popf

        /*  Push the pointer to the Multiboot information structure. */
        pushl   %ebx
        /*  Push the magic value. */
        pushl   %eax

        /*  Now enter the C main function... */
        call    boot_main
        
loop:   hlt
        jmp     loop
        .section ".init.data", "w"
stack:
        .space  STACK_SIZE
