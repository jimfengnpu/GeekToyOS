#ifndef X86_MM_H
#define X86_MM_H
#include <paging.h>
void map_region(pgd_t *pgdir, addr_t vaddr, addr_t paddr, u32 size, int perm, int large_table);

#endif