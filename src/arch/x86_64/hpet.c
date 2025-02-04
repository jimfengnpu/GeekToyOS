#include <interrupt.h>
#include <drivers/acpi.h>
#include <kernel/clock.h>
#include <kernel/mm.h>

#define HPET_REG_CAP    0x0
#define HPET_REG_CNF    0x10
#define HPET_REG_IS     0x20
#define HPET_REG_COUNTER    0xF0
#define HPET_REG_TIMER(n) ((n)*0x20 + 0x100)

#define HPET_TIMER_CNF_CAP  0x0
#define HPET_TIMER_CMP      0x8
#define HPET_TIMER_FSB      0x10

#define HPET_CAP_64COUNTER  0x2000
#define HPET_CAP_LEGACY     0x8000
#define HPET_CAP_MAX_TIMER(cap) (((cap)>>8) & 0x1f)
#define HPET_CAP_PERIOD(cap) ((cap) >> 32)

#define HPET_CNF_ENABLE     1
#define HPET_CNF_LEGACY     2

#define HPET_TIMER_CNF_TYPE_LEVEL   2
#define HPET_TIMER_CNF_TYPE_EDGE    0
#define HPET_TIMER_CNF_ENABLE       4
#define HPET_TIMER_CNF_DISABLE      0
#define HPET_TIMER_CNF_PERIOD       8
#define HPET_TIMER_CNF_SET          0x40
#define HPET_TIMER_CNF_32MODE       0x100
#define HPET_TIMER_CNF_FSB          0x4000
#define HPET_TIMER_CNF_INT(irq) (((irq)&0x1f) << 9)
#define HPET_TIMER_CNF_MASK         0x7F4E

#define HPET_TIMER_CAP_PERIOD       0x10
#define HPET_TIMER_CAP_64SZ         0x20
#define HPET_TIMER_CAP_INT(irq) (1L << ((irq) + 32))


static addr_t hpet_base;
static size_t num_timers;
static int hpet_cap_64;

struct address_structure
{
    u8 address_space_id;    // 0 - system memory, 1 - system I/O
    u8 register_bit_width;
    u8 register_bit_offset;
    u8 reserved;
    u64 address;
} __packed__;

struct hpet
{
    struct acpi_header header;
    u8 hardware_rev_id;
    u8 comparator_count:5;
    u8 counter_size:1;
    u8 reserved:1;
    u8 legacy_replacement:1;
    u16 pci_vendor_id;
    struct address_structure address;
    u8 hpet_number;
    u16 minimum_tick;
    u8 page_protection;
} __packed__;

u64 hpet_read(u32 reg) {
    return *((u64*)(hpet_base + reg));
}

void hpet_write(u32 reg, u64 val) {
    *((u64*)(hpet_base + reg)) = val;
    u64 _v = hpet_read(reg);
    if(_v != val) {
        info("hpet: write %x reg %lx failed:%lx\n", reg, val, _v);
    }
}

void hpet_write32(u32 reg, u32 val) {
    *((u32*)(hpet_base + reg)) = val;
    u64 _v = hpet_read(reg);
    if(_v&0xFFFFFFFF != val) {
        info("hpet: write %x reg %lx failed:%lx\n", reg, val, _v);
    }
}

int hpet_init()
{
    struct hpet *header = acpi_find_table("HPET");
    if(header == NULL){
        return 0;
    } 
    hpet_base = kaddr(header->address.address);
    arch_map_region(NULL, hpet_base, paddr(hpet_base), PGSIZE, PTE_W|PTE_G);
    u64 cap = hpet_read(HPET_REG_CAP);
    size_t period = HPET_CAP_PERIOD(cap);
    size_t target_cmp = 1000000000000000UL /(period * HZ);
    hpet_cap_64 = !!(HPET_CAP_64COUNTER & cap);
    num_timers = HPET_CAP_MAX_TIMER(cap);
    int valid_timer = -1;
    hpet_write(HPET_REG_CNF, 0);
    hpet_write(HPET_REG_COUNTER, 0);
    for(size_t i = 0; i <= num_timers; i++)
    {
        u64 timer_cnf = hpet_read(HPET_REG_TIMER(i) + HPET_TIMER_CNF_CAP);
        if ((timer_cnf&HPET_TIMER_CAP_PERIOD)
         && (timer_cnf& HPET_TIMER_CAP_INT(IRQ_TO_GSI(CLOCK_IRQ)))
         && (valid_timer == -1)){
            hpet_write(HPET_REG_TIMER(i) + HPET_TIMER_CNF_CAP, 
                (timer_cnf & (~HPET_TIMER_CNF_MASK)) | HPET_TIMER_CNF_PERIOD | HPET_TIMER_CNF_ENABLE | HPET_TIMER_CNF_SET |
                HPET_TIMER_CNF_TYPE_EDGE | HPET_TIMER_CNF_INT(IRQ_TO_GSI(CLOCK_IRQ)));
            hpet_write32(HPET_REG_TIMER(i) + HPET_TIMER_CMP, target_cmp);
            hpet_write32(HPET_REG_TIMER(i) + HPET_TIMER_CMP, target_cmp);
            valid_timer = i;
            continue;
        }
        hpet_write(HPET_REG_TIMER(i) + HPET_TIMER_CNF_CAP, 
            hpet_read(HPET_REG_TIMER(i) + HPET_TIMER_CNF_CAP) & (~HPET_TIMER_CNF_ENABLE));
    }
    return 1;
}

void hpet_start()
{
    hpet_write(HPET_REG_CNF, HPET_CNF_ENABLE|HPET_CNF_LEGACY);
}