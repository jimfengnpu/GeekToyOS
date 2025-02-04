#include <lib/string.h>
#include <drivers/acpi.h>
#include <kernel/multiboot2.h>


struct acpi_header* acpi_sdt_header;
static int acpi_revision;

int acpi_init()
{
    struct multiboot_tag_new_acpi *tag = mboot_get_mboot_info(MULTIBOOT_TAG_TYPE_ACPI_NEW);
    if (tag == NULL) {
        tag = mboot_get_mboot_info(MULTIBOOT_TAG_TYPE_ACPI_OLD);
    }
    if (tag == NULL) {
        error("no acpi\n");
        return 0;
    }
    struct rsdp* rsdp = (struct rsdp*)tag->rsdp;
    acpi_revision = rsdp->revision;
    if (rsdp->revision == 0) {
        acpi_sdt_header = kaddr(rsdp->rsdt_address);
    }else {
        acpi_sdt_header = kaddr(rsdp->xsdt_address);
    }
    return 1;
}

#define for_ptr_entry(type, ptr, start, end) \
    for(type* ptr = start; ptr < end; ptr++)
static struct acpi_header *acpi_iter(int (*acpi_handler)(struct acpi_header*, const void*), const void * arg)
{
    addr_t table_start = (u8*)acpi_sdt_header + sizeof(struct acpi_header);
    addr_t table_end = (u8*)acpi_sdt_header + acpi_sdt_header->length;
    if (acpi_revision) {
        for_ptr_entry(u64, entry, table_start, table_end){
            struct acpi_header *table = kaddr(*entry);
            if(acpi_handler(table, arg)){
                return table;
            }
        }
    }else {
        for_ptr_entry(u32, entry, table_start, table_end){
            struct acpi_header *table = kaddr(*entry);
            if(acpi_handler(table, arg)){
                return table;
            }
        }
    }
    return NULL;
}

static int __acpi_check_signature(struct acpi_header *header, const void *signature)
{
    return memcmp(header->signature, signature, 4) == 0;
}

struct acpi_header *acpi_find_table(const char *signature)
{
    if(acpi_sdt_header == NULL){
        return NULL;
    }
    return acpi_iter(__acpi_check_signature, signature);
}

static int __acpi_info(struct acpi_header *header, const void* __unused)
{
    info("ACPI: %c%c%c%c\n", header->signature[0],header->signature[1],header->signature[2],header->signature[3]);
    return 0;
}

void acpi_list()
{
    if(acpi_sdt_header == NULL){
        return NULL;
    }
    acpi_iter(__acpi_info, NULL);
}