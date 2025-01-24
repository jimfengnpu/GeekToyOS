#include <paging.h>
#include <boot.h>
#include <kernel/kernel.h>

pgd_t *kernel_pgd;

void map_region(pgd_t *pgdir, addr_t vaddr, addr_t paddr, u32 size, int perm, int large_table)
{

}
