#include <kernel/kernel.h>
#include <cpu.h>

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