#ifndef X86_MM_H
#define X86_MM_H
#include <paging.h>
void arch_map_region(pgd_t *pgdir, addr_t vaddr, addr_t paddr, u32 size, int perm);
void arch_map_kernel_page(addr_t max_phy_addr);
addr_t arch_kmap(addr_t phy, size_t sz);
void arch_kunmap(addr_t va);
#endif