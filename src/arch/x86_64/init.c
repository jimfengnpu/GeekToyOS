#include <kernel/kernel.h>
#include <kernel/multiboot2.h>
#include <cpu.h>
#include <mm.h>
#include <interrupt.h>


extern char pgd[];
extern pgd_t *kernel_pgd; // in x86_64/mm.c

char fxsave_region[512] __attribute__((aligned(16)));


static void check_enable_sse()
{
    if(check_cpu_feature(FEAT_X86_SSE2))
    {
        asm volatile("movq %cr0, %rax");
        asm volatile("andw %0, %%ax"::"i"(0xFFFB));//clear coprocessor emulation CR0.EM
        asm volatile("orw %0, %%ax"::"i"(0x2));//set coprocessor monitoring  CR0.MP
        asm volatile("movq %rax, %cr0");
        asm volatile("movq %cr4, %rax");
        asm volatile("or %0, %%ax"::"i"(3 << 9)); //set CR4.OSFXSR and CR4.OSXMMEXCPT at the same time
        asm volatile("mov %rax, %cr4");
        asm volatile("fxsave %0 "::"m"(fxsave_region));
    }
}

void boot_start()
{
    fetch_cpu_feature();
    // check_enable_sse();
    kernel_main();
}

void ap_start()
{
    // check_enable_sse();
    ap_main();
}

__init_text void boot_main(u32 magic, u32 mboot_info_phy)
{
    mboot_info = (struct multiboot_tag*)kaddr(mboot_info_phy);
    kernel_pgd = (pgd_t*)kaddr(pgd);
    boot_start();
}

extern volatile int smp_ap_started_flag;
__init_text void boot_ap()
{
    smp_ap_started_flag = 1;
    ap_start();
}