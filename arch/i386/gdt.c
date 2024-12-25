#include <kernel/kernel.h>
#include <gdt.h>
// 全局描述符表长度
#define GDT_LENGTH 6


// Define GDT entries
descriptor_t gdt[GDT_LENGTH];
gdt_ptr gp;
tss_t tss[1];
// Function to set a GDT entry
void gdt_set_gate(int num, u32 base, u32 limit, u16 attr) {
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].limit_high_attr2 = (limit >> 16) & 0x0F;

    gdt[num].limit_high_attr2 |= (attr >> 8) & 0xF0;
    gdt[num].attr1 = attr & 0xFF;
}

// Function to initialize the GDT
void init_gdt() {
    gp.limit = (sizeof(descriptor_t) * GDT_LENGTH) - 1;
    gp.base = (u32)&gdt;
    tss[0].ts_ss0 = KERNEL_CS;
    tss[0].ts_iomb = sizeof(tss_t);
    gdt_set_gate(SEG_NULL, 0, 0, 0);                       // Null segment
    gdt_set_gate(SEG_KTEXT, 0, 0xFFFFF, DA_CR | DA_32 | DA_LIMIT_4K | DA_DPL0);
    gdt_set_gate(SEG_KDATA, 0, 0xFFFFF, DA_DRW | DA_32 | DA_LIMIT_4K | DA_DPL0);
    gdt_set_gate(SEG_UTEXT, 0, 0xFFFFF, DA_CR | DA_32 | DA_LIMIT_4K | DA_DPL3);
    gdt_set_gate(SEG_UDATA, 0, 0xFFFFF, DA_DRW | DA_32 | DA_LIMIT_4K | DA_DPL3);
    gdt_set_gate(SEG_TSS, (addr_t)tss, sizeof(tss_t)-1, DA_386TSS);
    asm volatile("cli");
    // Load the GDT
    asm volatile("lgdt (%0)" : : "m" (gp));

    // Update segment registers
    asm volatile(
        "movw %0, %%ax\n"
        "movw %%ax, %%ds\n"
        "movw %%ax, %%es\n"
        "movw %%ax, %%fs\n"
        "movw %%ax, %%gs\n"
        "movw %%ax, %%ss\n"::"i"(GD_KDATA)
    );
    asm volatile(
        "ljmp %0,$_next\n"
        "_next:\n":: "i"(GD_KTEXT)
    );

    // Load TSS 
    asm volatile(
        "mov %0, %%ax\n"
        "ltr %%ax"::"i"(GD_TSS)
    );
}