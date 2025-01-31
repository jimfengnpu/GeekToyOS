#ifndef X86_MM_H
#define X86_MM_H
#include <paging.h>
void arch_map_region(pgd_t *pgdir, addr_t vaddr, addr_t paddr, size_t size, int perm);
void arch_map_kernel_page(addr_t max_phy_addr);
#endif