#pragma once
#include <drivers/acpi.h>

#define MAX_LAPICS MAX_CORES
#define MAX_IOAPICS 16
#define MAX_NMIS (2 * MAX_LAPICS)
#define MAX_OVERRIDES 48

#define IPI_INIT     0x4500
#define IPI_START_UP 0x4600
#define IPI_FIXED    0x4000

#define IPI_BROADCAST (0b11 << 18)

#define APIC_MSR	0x1B
#define APIC_MSR_ENABLE	0x800

struct acpi_madt {
	struct acpi_header header;
	u32 lapic_address;
	u32 flags;
	u8 entries[];
} __packed__;

struct madt_entry_header {
	u8 type;
	u8 length;
} __packed__;

#define MADT_LAPIC 0
struct madt_entry_lapic {
	struct madt_entry_header header;
	u8 acpi_id;
	u8 apic_id;
	u32 flags;
} __packed__;

#define MADT_IOAPIC 1
struct madt_entry_ioapic {
	struct madt_entry_header header;
	u8 apic_id;
	u8 __zero;
	u32 phys_addr;
	u32 gsi_base;
} __packed__;

#define MADT_OVERRIDE 2
struct madt_entry_override {
	struct madt_entry_header header;
	u8 bus; // Constant, set to 0
	u8 source;
	u32 gsi;
	u16 flags;
} __packed__;

#define MADT_NMI 4
struct madt_entry_nmi {
	struct madt_entry_header header;
	u8 acpi_id;
	u16 flags;
	u8 lint_num;
} __packed__;

#define MADT_LAPIC_ADDR 5
struct madt_entry_lapic_addr {
	struct madt_entry_header header;
	u16 __zero;
	u64 lapic_addr;
} __packed__;

struct lapic_info {
	uint8_t id;
	uint8_t acpi_id;
	uint8_t present;
	uint32_t ticks_per_10ms;
};

int apic_init(void);

void lapic_eoi(u8);
void ioapic_mask(u32 gsi);
void ioapic_unmask(u32 gsi);