#include <kernel/kernel.h>
#include <kernel/multiboot2.h>
#include <mm.h>
#include <interrupt.h>


extern char pgd[];
extern pgd_t *kernel_pgd; // in x86_64/mm.c
struct cpu_feature feature;

void fetch_cpu_feature()
{
    u32 ecx, edx;
    cpuid(0x1, NULL, NULL, &ecx, &edx);
    feature.feature = (((u64)ecx)<< 32)|edx;
    cpuid(0x80000001, NULL, NULL, &ecx, &edx);
    feature.extend_feature = (((u64)ecx)<< 32)|edx;
}

int check_cpu_feature(int type)
{
    u64 mask = 1UL;
    if(type >= 64){
        mask <<= type - 64;
        return feature.extend_feature & mask;
    }else{
        mask <<= type;
        return feature.feature & mask;
    }
}

void boot_start()
{
    fetch_cpu_feature();
    // clear low page
    kernel_pgd[0] = 0;
    kernel_main();
}

__init_text void boot_main(u32 magic, u32 mboot_info_phy)
{
    mboot_info = (struct multiboot_tag*)kaddr(mboot_info_phy);
    kernel_pgd = (pgd_t*)kaddr(pgd);
    boot_start();
}