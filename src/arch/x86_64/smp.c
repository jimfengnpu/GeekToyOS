#include <kernel/kernel.h>
#include <kernel/mm.h>
#include <kernel/clock.h>
#include <lib/string.h>
#include <atomic.h>
#include <apic.h>

extern char mp_start[];
extern char mp_end[];
#define MP_BASE			0x8000 // same as boot_mp.S
volatile int smp_ap_started_flag;
volatile u8* mp_stacktop;

extern struct lapic_info lapic_list[MAX_LAPICS];

static int smp_cpu_ready_count;

u8 smp_cpunum(void){
    return lapic_num();
}

u8 smp_cpuid(void){
    u8 id = lapic_id();
	for(u8 i = 0; i < lapic_num(); i++){
		if(lapic_list[i].id == id){
			return i;
		}
	}
	panic("lapic id not found!");
}

static void smp_boot(u8 id)
{
    struct lapic_info *lapic = &lapic_list[id];

	if (mp_stacktop == NULL) {
		addr_t stack = kaddr(mm_phy_page_alloc(1));
		mp_stacktop = (addr_t)(stack + (PGSIZE));
	}

	// Set by the AP when initialisation is complete
	smp_ap_started_flag = 0;

	// Adapted from https://nemez.net/osdev/lapic.txt
	// Send the INIT IPI
	clock_sleep(1);
	lapic_send_ipi(lapic->id, IPI_INIT);
	clock_sleep(1);

	// Send the SIPI (first attempt)
	lapic_send_ipi(lapic->id, IPI_START_UP | ((u32)MP_BASE / PGSIZE));
	clock_sleep(1);

	if (!smp_ap_started_flag) {
		// Send SIPI again (second attempt)
		lapic_send_ipi(lapic->id, IPI_START_UP | ((u32)MP_BASE / PGSIZE));
        clock_sleep_watch_flag(1, &smp_ap_started_flag, 0, 1);
		if (!smp_ap_started_flag) {
			klog("SMP: cpu %d failed to boot\n", id);
			lapic->present = 0;
			return;
		}
	}

	mp_stacktop = NULL;
}

void smp_init()
{
    if (lapic_num() == 1) {
        klog("SMP: no mp, fallback to single cpu\n");
        return;
    }
	klog("SMP: cpu %d online\n", smp_cpuid());
    arch_map_region(NULL, MP_BASE, MP_BASE, mp_end - mp_start, PTE_W);
    memcpy(MP_BASE, mp_start, mp_end - mp_start);
    for(u8 i = 1; i < smp_cpunum(); i++)
    {
        smp_boot(i);
    }
    arch_map_region(NULL, MP_BASE, 0, mp_end - mp_start, PG_CLEAR);
    arch_clear_low_page();
	smp_sync_ready();
	info("SMP: %d cpu online\n", smp_cpu_ready_count);
}

void smp_sync_ready()
{
	klog("SMP: cpu %d online\n", smp_cpuid());
	atomic_inc(&smp_cpu_ready_count);
	clock_sleep_watch_flag(2, &smp_cpu_ready_count, smp_cpunum(), 0);
}