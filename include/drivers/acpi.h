#ifndef ACPI_H
#define ACPI_H
#include <kernel/kernel.h>
#include <lib/types.h>

struct acpi_header {
    char signature[4];
    u32 length;
    u8  revision;
    u8  checksum;
    char oem_id[6];
    char oem_table_id[8];
    u32 oem_revision;
    u8  creator_id[4];
	u32 creator_revision;
} __packed__;


struct rsdp {
	char signature[8];
	u8 checksum;
	char oem_id[6];
	u8 revision;
	u32 rsdt_address;
	u32 length;
	u64 xsdt_address;
	u8 ext_checksum;
	u8 reserved[3];
} __packed__;

struct rsdt {
	struct acpi_header header;
	u32 tables[];
} __packed__;

struct xsdt {
	struct acpi_header header;
	addr_t tables[];
} __packed__;

int acpi_init();
struct acpi_header *acpi_find_table(const char *signature);
void acpi_list();
#endif