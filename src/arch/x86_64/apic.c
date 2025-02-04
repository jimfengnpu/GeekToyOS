#include <apic.h>
#include <kernel/interrupt.h>
#include <kernel/clock.h>
#include <kernel/mm.h>

#define APIC_CPUID_BIT (1 << 9)
#define APIC_TIMER_PERIODIC 0x20000U
#define APIC_DISABLE 0x10000U

#define APIC_REG_ID 0x20
#define APIC_REG_VERSION 0x30
#define APIC_REG_EOI 0xB0
#define APIC_REG_SPURIOUS 0xF0U
#define APIC_REG_LINT0 0x350U
#define APIC_REG_LINT1 0x360U
#define APIC_REG_ICR0 0x300U
#define APIC_REG_ICR1 0x310U
#define APIC_REG_TIMER_LVT 0x320U
#define APIC_REG_TIMER_INITIAL 0x380U
#define APIC_REG_TIMER_CURRENT 0x390U
#define APIC_REG_TIMER_DIVIDE 0x3E0U
#define APIC_REG_ISR_BASE 0x100U

struct lapic_info lapic_list[MAX_LAPICS];
struct madt_entry_ioapic *ioapic_list[MAX_IOAPICS];
struct madt_entry_override *override_list[MAX_OVERRIDES];
struct madt_entry_nmi *nmi_list[MAX_NMIS];
size_t lapic_list_size;
size_t ioapic_list_size;
size_t override_list_size;
size_t nmi_list_size;
addr_t lapic_base;

static void add_lapic(struct madt_entry_lapic *entry)
{
	if (entry->flags == 0)
		return;
	if (lapic_list_size >= MAX_LAPICS)
		return;
	lapic_list[lapic_list_size].present = (entry->flags == 1);
	lapic_list[lapic_list_size].id = entry->apic_id;
	lapic_list[lapic_list_size].acpi_id = entry->acpi_id;
	lapic_list_size++;
	// klog("APIC:Detected Local APIC, id %d\n", entry->apic_id);
}

static void add_ioapic(struct madt_entry_ioapic *entry)
{
	if (ioapic_list_size >= MAX_IOAPICS)
		return;
	ioapic_list[ioapic_list_size++] = entry;
    arch_map_region(NULL, kaddr(entry->phys_addr), entry->phys_addr, PGSIZE, PTE_W|PTE_G);
	klog("APIC: Detected I/O APIC at %p, id %d\n", (void *)(addr_t)kaddr(entry->phys_addr), entry->apic_id);
}

static void add_override(struct madt_entry_override *entry)
{
	if (override_list_size >= MAX_OVERRIDES)
		return;
	override_list[override_list_size++] = entry;
	// klog("APIC:GSI %d overrides IRQ %u, flags %x\n", entry->gsi, entry->source, entry->flags);
}

static void add_nmi(struct madt_entry_nmi *entry)
{
	if (nmi_list_size >= MAX_NMIS)
		return;
	nmi_list[nmi_list_size++] = entry;
	// if (entry->acpi_id == 0xFF)
	// 	klog("APIC: NMI for all CPUs, LINT%d\n", entry->lint_num);
	// else
	// 	klog("APIC: NMI for CPU %d, LINT%d\n", entry->acpi_id, entry->lint_num);
}

static void parse_madt(struct acpi_madt *madt)
{
	// Default LAPIC address (might be overridden by entry type 5)
	addr_t tmp_lapic_base = kaddr(madt->lapic_address);

	// Parse all the other entries
	struct madt_entry_header *hd = (struct madt_entry_header *)&madt->entries;
	struct madt_entry_header *end = (struct madt_entry_header *)((addr_t)madt + madt->header.length);

	while (hd < end) {
		switch (hd->type) {
			case MADT_LAPIC:
				add_lapic((struct madt_entry_lapic *)hd);
				break;
			case MADT_IOAPIC:
				add_ioapic((struct madt_entry_ioapic *)hd);
				break;
			case MADT_OVERRIDE:
				add_override((struct madt_entry_override *)hd);
				break;
			case MADT_NMI:
				add_nmi((struct madt_entry_nmi *)hd);
				break;
			case MADT_LAPIC_ADDR:
				tmp_lapic_base = kaddr(((struct madt_entry_lapic_addr *)hd)->lapic_addr);
				break;
			default:
				warning("APIC: Unrecognised entry type %d in MADT\n", hd->type);
				break;
		}
		hd = (struct madt_entry_header *)((uintptr_t)hd + hd->length);
	}

	lapic_base = tmp_lapic_base;
	klog("APIC: Local APIC base: %p, Detected %d CPUs\n", 
		lapic_base, lapic_list_size);
    arch_map_region(NULL, lapic_base, paddr(lapic_base), PGSIZE, PTE_W|PTE_G);
}

int apic_init(void)
{
    if(check_cpu_feature(FEAT_X86_APIC) == 0){
        return 0;
    }
	struct acpi_madt *madt = acpi_find_table("APIC");
	if (madt == NULL){
		error("no MADT found in ACPI tables");
		return 0;
	}
	parse_madt(madt);
	lapic_enable();
	ioapic_init();
	return 1;
}

static inline void lapic_write(u32 reg_offset, u32 data)
{
	*(volatile u32 *)((uintptr_t)lapic_base + reg_offset) = data;
}

static inline u32 lapic_read(u32 reg_offset)
{
	return *(volatile u32 *)((uintptr_t)lapic_base + reg_offset);
}

static inline struct lapic_info *find_lapic(uint8_t id)
{
	for (size_t i = 0; i < lapic_list_size; i++)
		if (lapic_list[i].id == id)
			return &lapic_list[i];
	return NULL;
}

static inline void lapic_set_nmi(uint8_t vec, struct madt_entry_nmi *nmi_info)
{
	u32 nmi = 800 | vec;
	if (nmi_info->flags & 2)
		nmi |= (1 << 13);
	if (nmi_info->flags & 8)
		nmi |= (1 << 15);
	if (nmi_info->lint_num == 0)
		lapic_write(APIC_REG_LINT0, nmi);
	else if (nmi_info->lint_num == 1)
		lapic_write(APIC_REG_LINT1, nmi);
}

u8 lapic_id(void)
{
	return lapic_read(APIC_REG_ID) >> 24;
}

// Returns the number of ticks in 10ms
u32 lapic_timer_prepare(void)
{
	const u32 test_ms = 50;
	const u32 ticks_initial = 0xFFFFFFFF;
	
	lapic_write(APIC_REG_TIMER_DIVIDE, 0x3);
	lapic_write(APIC_REG_TIMER_INITIAL, ticks_initial);
	clock_sleep(test_ms * HZ / 1000);
	// pit_sleep_ms(test_ms); // TODO: Use interrupts for better accuracy (might be tricky getting interrupts with SMP)

	lapic_write(APIC_REG_TIMER_LVT, APIC_DISABLE);

	u32 ticks_per_10ms = (ticks_initial - lapic_read(APIC_REG_TIMER_CURRENT)) / (test_ms / 10);
	// u32 ticks_per_10ms = 625000;
	klog("LAPIC:test tick/10ms:%d\n", ticks_per_10ms);
	
	return ticks_per_10ms;
}

void lapic_timer_enable(void)
{
	u8 id = lapic_id();
	struct lapic_info *lapic = find_lapic(id);

	const u32 period = lapic_timer_prepare();
	lapic->ticks_per_10ms = period;

	lapic_write(APIC_REG_TIMER_DIVIDE, 0x3);
	lapic_write(APIC_REG_TIMER_LVT, IRQ_LINT_TIMER | APIC_TIMER_PERIODIC);
	lapic_write(APIC_REG_TIMER_INITIAL, period);
}

void sched_handler(trapframe_t *frame);

void lapic_enable(void)
{
	msr_write(APIC_MSR, paddr(lapic_base) | APIC_MSR_ENABLE);
	u8 id = lapic_id();
	struct lapic_info *lapic = find_lapic(id);

	for (size_t i = 0; i < nmi_list_size; i++)
		if (nmi_list[i]->acpi_id == lapic->acpi_id || nmi_list[i]->acpi_id == 0xFF)
			lapic_set_nmi(IRQ_NMI, nmi_list[i]);

	// Enable the LAPIC via the spurious interrupt register
	lapic_write(APIC_REG_SPURIOUS, lapic_read(APIC_REG_SPURIOUS) | (1 << 8) | IRQ_SPURIOUS);

	// Register timer callback (only do this once)
	if (id == 0) {
		register_interrupt_handler(IRQ_LINT_TIMER, ISR_IRQ, sched_handler);
	}

	// Clear any pending interrupts
	lapic_write(APIC_REG_EOI, 0);
}


void lapic_eoi(u8 vec)
{
	u32 reg = APIC_REG_ISR_BASE + 0x10 * (vec / 32);
	if (lapic_read(reg) & (1 << (vec % 32)))
		lapic_write(APIC_REG_EOI, 0);

}

void lapic_send_ipi(uint8_t target, u32 flags)
{
	if (lapic_base == NULL)
		panic("Tried to send IPI before LAPIC was initialized");
	
	if (!(flags & IPI_BROADCAST))
		lapic_write(APIC_REG_ICR1, (u32)target << 24);
	lapic_write(APIC_REG_ICR0, flags);
}


#define IOAPIC_REG_ID 0
#define IOAPIC_REG_VER 1
#define IOAPIC_REG_REDTBL 16

#define APIC_IRQ_MASK 0x10000


// Careful: will race
static inline u32 ioapic_read(volatile u32 *ioapic, u8 reg)
{
	ioapic[0] = (reg & 0xFF);
	return ioapic[4];
}

// Careful: will race
static inline void ioapic_write(volatile u32 *ioapic, u8 reg, u32 data)
{
	ioapic[0] = (reg & 0xFF);
	ioapic[4] = data;
}

static inline u32 get_max_redirs(size_t ioapic_id)
{
	addr_t base = kaddr(ioapic_list[ioapic_id]->phys_addr);
	return (ioapic_read(base, IOAPIC_REG_VER) & 0xFF0000) >> 16;
}

static struct madt_entry_ioapic *gsi_to_ioapic(u32 gsi)
{
	for (size_t i = 0; i < ioapic_list_size; i++) {
		u32 max_redirs = get_max_redirs(i);
		if (ioapic_list[i]->gsi_base <= gsi && ioapic_list[i]->gsi_base + max_redirs > gsi)
			return ioapic_list[i];
	}
	panic("I/O APIC not found for GSI %u", gsi);
}

static u64 ioapic_redtbl_read(addr_t ioapic_base, u8 irq_line)
{
	u32 ioredtbl = (irq_line * 2) + IOAPIC_REG_REDTBL;
	return ioapic_read(ioapic_base, ioredtbl) | ((u64)ioapic_read(ioapic_base, ioredtbl + 1) << 32);
}

static void ioapic_redtbl_write(addr_t ioapic_base, u8 irq_line, u64 value)
{
	u32 ioredtbl = (irq_line * 2) + IOAPIC_REG_REDTBL;
	ioapic_write(ioapic_base, ioredtbl + 0, (u32)value);
	ioapic_write(ioapic_base, ioredtbl + 1, (u32)(value >> 32));
}

void ioapic_redirect(u32 gsi, u8 source, u16 flags, u8 target_apic)
{
	u8 target_apic_id = lapic_list[target_apic].id;
	struct madt_entry_ioapic *ioapic = gsi_to_ioapic(gsi);
	addr_t ioapic_base = kaddr(ioapic->phys_addr);

	u64 redirection = gsi + IRQ_BASE;
	if (flags & 2)
		redirection |= (1 << 13);
	if (flags & 8)
		redirection |= (1 << 15);
	redirection |= ((u64)target_apic_id) << 56;

	ioapic_redtbl_write(ioapic_base, gsi - ioapic->gsi_base, redirection);
}

void ioapic_mask(u32 gsi)
{
	// spin_lock(&ioapic_lock);
	struct madt_entry_ioapic *ioapic = gsi_to_ioapic(gsi);
	addr_t ioapic_base = kaddr(ioapic->phys_addr);
	u8 irq_line = gsi - ioapic->gsi_base;
	u64 prev = ioapic_redtbl_read(ioapic_base, irq_line);
	ioapic_redtbl_write(ioapic_base, irq_line, prev | APIC_IRQ_MASK);
	// spin_unlock(&ioapic_lock);
}

void ioapic_unmask(u32 gsi)
{
	// spin_lock(&ioapic_lock);
	struct madt_entry_ioapic *ioapic = gsi_to_ioapic(gsi);
	addr_t ioapic_base = kaddr(ioapic->phys_addr);
	u8 irq_line = gsi - ioapic->gsi_base;
	u64 prev = ioapic_redtbl_read(ioapic_base, irq_line);
	ioapic_redtbl_write(ioapic_base, irq_line, prev & (~APIC_IRQ_MASK));
	// spin_unlock(&ioapic_lock);
}

u8 ioapic_isa_to_gsi(u8 isa)
{
	for (size_t i = 0; i < override_list_size; i++)
		if (override_list[i]->source == isa)
			return override_list[i]->gsi;
	return isa;
}

u8 ioapic_gsi_to_isa(u8 gsi)
{
	for (size_t i = 0; i < override_list_size; i++)
		if (override_list[i]->gsi == gsi)
			return override_list[i]->source;
	return gsi;
}

void ioapic_init(void)
{
	// Initialised the (assumed) wirings for the legacy PIC IRQs
	// Send all IRQs to the BSP for simplicity
	for (u8 i = 0; i < 16; i++) {
		ioapic_redirect(i, i, 0, 0);
		ioapic_mask(i);
	}

	// Setup the actual overrides
	for (size_t i = 0; i < override_list_size; i++) {
		ioapic_redirect(override_list[i]->gsi, override_list[i]->source, override_list[i]->flags, 0);
		ioapic_mask(override_list[i]->gsi);
	}
}