#include <paging.h>
#include <boot.h>
#include <kernel/kernel.h>
#include <kernel/mm.h>
#include <lib/bitmap.h>

struct bitmap high_kmap;
pgd_t *kernel_pgd;
// flag:
#define PG_ALLOC    1

static  pte_t * pgdir_walk(pgd_t *pgdir, addr_t vaddr, addr_t *paddr, int flag)
{
    u32 pdx = PDX(vaddr), ptx = PTX(vaddr);
    u32 pde_attr = PTE_P | PTE_W;
    int alloc = flag & PG_ALLOC;
    pde_t *pde = NULL;
    if (!(pgdir[pdx] & PTE_P)) {
        if (!alloc){
            return NULL;
        }
        pgdir[pdx] = mm_phy_page_alloc(1) | pde_attr;
    }
    pde = kaddr(PTADDR(pgdir[pdx]));
    if(paddr){
        *paddr = ((pde[ptx] & (~bit_mask(PTXSHIFT))) | (vaddr & bit_mask(PTXSHIFT)));
    }
    return &pde[ptx];
}

addr_t check_pgtable(addr_t vaddr)
{
    addr_t addr = NULL;
    pte_t *pte = pgdir_walk(kernel_pgd, vaddr, &addr, 0);
    if(pte==NULL){
        debug("%lx=> no page\n", vaddr);
    }else{
        debug("%lx=> %lx\n", vaddr, addr);
    }
    return addr;
}

void arch_map_region(pgd_t *pgdir, addr_t vaddr, addr_t paddr, u32 size, int perm)
{
    if(pgdir == NULL){
        pgdir = kernel_pgd;
    }
    size_t pg_size = PGSIZE;// 0x1000
    addr_t vaddr_end = align(vaddr + size, pg_size);
    addr_t _vaddr = align_down(vaddr, pg_size);
    paddr = paddr - (vaddr - _vaddr);
    vaddr = _vaddr;
    int flag = PG_ALLOC;
    for(addr_t addr = vaddr; addr < vaddr_end;)
    {
        pte_t *pte = pgdir_walk(pgdir, addr, NULL, flag);
        // if(pte == NULL){
        // }
        *pte = paddr | PTE_P | perm;
        addr += pg_size;
        paddr += pg_size;
    }
}

#define PHY_BOOT_BASE 0x400000 // 4M page mapped in asm
#define PHY_ACCESS_LIMIT (KMAPPING_BASE - KERN_BASE)
#define HIGH_MEM_BITMAP_SIZE    65536
void arch_map_kernel_page(addr_t max_phy_addr)
{
    if (max_phy_addr <= PHY_BOOT_BASE){
        return;
    }
    max_phy_addr = min(max_phy_addr, PHY_ACCESS_LIMIT);
    arch_map_region(NULL, kaddr(PHY_BOOT_BASE), PHY_BOOT_BASE, max_phy_addr - PHY_BOOT_BASE, PTE_W|PTE_G);
    arch_map_region(NULL, KMAPPING_LIMIT, KMAPPING_LIMIT, 0x8000000, PTE_W|PTE_G);
    u8 *high_bitmap = kaddr(mm_phy_page_alloc((HIGH_MEM_BITMAP_SIZE/8)/PGSIZE));
    bitmap_init(&high_kmap, high_bitmap, HIGH_MEM_BITMAP_SIZE);
    bitmap_set(&high_kmap, 0, 0, HIGH_MEM_BITMAP_SIZE);
}

addr_t arch_kmap(addr_t phy, size_t sz) {
    addr_t va = NULL;
    size_t pgoff = PGOFF(phy);
    phy = PTADDR(phy);
    if(phy >= KMAPPING_LIMIT) {
        va = phy;
        return;
    }
    if(phy < PHY_ACCESS_LIMIT) {
        va = kaddr(phy);
    }else {
        size_t bit = div_round_up(sz, PGSIZE);
        ssize_t l = 0, r = 0;
        while(r < HIGH_MEM_BITMAP_SIZE)
        {
            l = bitmap_find(&high_kmap, 0, r, HIGH_MEM_BITMAP_SIZE);
            r = bitmap_find(&high_kmap, 1, l, bit);
            if(r == -1) {
                va = KMAPPING_BASE + (l << PGSHIFT);
                bitmap_set(&high_kmap, 1, l, bit);
                // va = check_pgtable(phy);
                break;
            }
        }
    }
    if(va == NULL){
        panic("no kmap addr\n");
    }
    if(check_pgtable(va) != phy)
        arch_map_region(NULL, va, phy, sz + pgoff, PTE_W);
    return va + pgoff;
}

void arch_kunmap(addr_t va) {
    if(va < KMAPPING_BASE) {
        return;
    }
    addr_t pa;
    bitmap_set(&high_kmap, 0, (va - KMAPPING_BASE)/PGSIZE, 1);
    pte_t *pte = pgdir_walk(kernel_pgd, va, &pa, 0);
    *pte = 0;
}