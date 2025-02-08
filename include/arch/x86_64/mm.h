#ifndef X86_MM_H
#define X86_MM_H
#include <paging.h>
#define PG_CLEAR    0x1000
void arch_map_region(pgd_t *pgdir, addr_t vaddr, addr_t paddr, size_t size, int perm_flag);
void arch_map_kernel_page(addr_t max_phy_addr);
addr_t arch_kmap(addr_t phy, size_t sz);
void arch_kunmap(addr_t va);
void arch_clear_low_page();
#endif