#include <paging.h>
#include <kernel/kernel.h>
#include <kernel/mm.h>

pgd_t *kernel_pgd;

// internal flag:
#define PG_ALLOC    4
#define PG_2M   1
#define PG_1G   2

static void expand_page(pde_t *pde, int table_level)
{
    addr_t phy = PTADDR(*pde);
    int attr = PGOFF(PGOFF(*pde) & (~PTE_PS));
    addr_t new_pg_phy = mm_phy_page_zalloc(1);
    *pde = new_pg_phy | (PTE_W|PTE_P);
    pde_t *new_pg = kaddr(new_pg_phy);
    size_t pg_size = PGSIZE;
    if(table_level){
        pg_size = PTSIZE;
    }
    for(size_t i = 0; i < NPDENTRIES; i++){
        new_pg[i] = (phy + pg_size*i)|((table_level?PTE_PS:0)|attr);
    }
}

static  pte_t * pgdir_walk(pgd_t *pgdir, addr_t vaddr, addr_t *paddr, int flag)
{
    u32 pgx= PGX(vaddr), pux = PUX(vaddr), pdx = PDX(vaddr), ptx = PTX(vaddr);
    u32 pde_attr = PTE_P | PTE_W;
    int alloc = flag & PG_ALLOC;
    int table_level = flag & 3;
    pde_t *pde = NULL;
    if (!(pgdir[pgx] & PTE_P)) {
        if (!alloc){
            return NULL;
        }
        pgdir[pgx] = mm_phy_page_zalloc(1) | pde_attr;
    }
    pde = kaddr(PTADDR(pgdir[pgx]));

    if (!(pde[pux] & PTE_P)) {
        if (!alloc){
            return NULL;
        }
        if (flag & PG_1G) {
            return &pde[pux];
        }
        pde[pux] = mm_phy_page_zalloc(1) | pde_attr;
    } else if((pde[pux] & PTE_PS)) {
        if (!alloc) {
            if(paddr){
                *paddr = ((pde[pux] & (~bit_mask(PUXSHIFT))) | (vaddr & bit_mask(PUXSHIFT)));
            }
            return &pde[pux];
        }
        expand_page(&pde[pux], 1);
    } else if((flag & PG_1G) && alloc){
        warning("try to shrink page, page table phy mem free:");
        mm_phy_page_free(PTADDR(pde[pux]), 1);
        return &pde[pux];
    }
    pde = kaddr(PTADDR(pde[pux]));

    if (!(pde[pdx] & PTE_P)) {
        if (!alloc){
            return NULL;
        }
        if (flag & PG_2M) {
            return &pde[pdx];
        }
        pde[pdx] = mm_phy_page_zalloc(1) | pde_attr;
    } else if ((pde[pdx] & PTE_PS)) {
        if (!alloc) {
            if(paddr){
                *paddr = ((pde[pdx] & (~bit_mask(PDXSHIFT))) | (vaddr & bit_mask(PDXSHIFT)));
            }
            return &pde[pdx];
        }
        expand_page(&pde[pdx], 0);
    }else if((flag & PG_2M) && alloc){
        warning("try to shrink page, page table phy mem free:");
        mm_phy_page_free(PTADDR(pde[pdx]), 1);
        return &pde[pdx];
    }

    pde = kaddr(PTADDR(pde[pdx]));
    if(paddr){
        *paddr = ((pde[ptx] & (~bit_mask(PTXSHIFT))) | (vaddr & bit_mask(PTXSHIFT)));
    }
    return &pde[ptx];
}

addr_t check_pgtable(addr_t vaddr)
{
    addr_t addr = NULL;
    pte_t *pte = pgdir_walk(kernel_pgd, vaddr, &addr, 0);
    // if(pte==NULL){
    //     debug("%lx=> no page\n", vaddr);
    // }else {
    //     debug("%lx=>%lx\n", vaddr, addr);
    // }
    return addr;
}

static void __map_region(pgd_t *pgdir, addr_t vaddr, addr_t paddr, size_t size, int attr, int table_level)
{
    
    size_t pg_size = PGSIZE;// 0x1000
    if(table_level == 1)pg_size = PTSIZE; // 0x20_0000
    else if(table_level == 2)pg_size = PTSIZE*NPDENTRIES; // 0x4000_0000
    if(!size){
        return;
    }
    addr_t vaddr_end = align(vaddr + size, pg_size);
    addr_t _vaddr = align_down(vaddr, pg_size);
    paddr = paddr - (vaddr - _vaddr);
    vaddr = _vaddr;
    int flag = PG_ALLOC|table_level;
    klog("%lx --- %lx,%d, %lx\n", vaddr, vaddr_end, table_level, paddr);
    for(addr_t addr = vaddr; addr < vaddr_end;)
    {
        pte_t *pte = pgdir_walk(pgdir, addr, NULL, flag);
        // if(pte == NULL){
        // }
        *pte = paddr | attr | ((table_level)?PTE_PS:0);
        addr += pg_size;
        paddr += pg_size;
    }
}

void arch_map_region(pgd_t *pgdir, addr_t vaddr, addr_t paddr, size_t size, int perm_flag)
{
    if(pgdir == NULL){
        pgdir = kernel_pgd;
    }
    addr_t va_s1, va_s2, va_e1, va_e2, delta, va_end;
    delta = vaddr - paddr;
    va_s1 = align(vaddr, PTSIZE);
    va_e1 = align_down(vaddr + size, PTSIZE);
    va_s2 = align(vaddr, PTSIZE * NPTENTRIES);
    va_e2 = align_down(vaddr + size, PTSIZE * NPTENTRIES);
    va_end = align(vaddr + size, PGSIZE);
    vaddr = align_down(vaddr, PGSIZE);
    int flag = perm_flag & (~PG_ATTR_MASK);
    int perm = perm_flag & (PG_ATTR_MASK);
    if(!(flag & PG_CLEAR)){
        perm |= PTE_P;
    }
    if (va_e1 > va_s1) {
        __map_region(pgdir, vaddr, vaddr - delta, va_s1 - vaddr, perm, 0);
        if (va_e2 > va_s2 && check_cpu_feature(FEAT_X86_Page1GB)) 
        {
            __map_region(pgdir, va_s1, va_s1 - delta, va_s2 - va_s1, perm, 1);
            __map_region(pgdir, va_s2, va_s2 - delta, va_e2 - va_s2, perm, 2);
            __map_region(pgdir, va_e2, va_e2 - delta, va_e1 - va_e2, perm, 1);
        } else {
            __map_region(pgdir, va_s1, va_s1 - delta, va_e1 - va_s1, perm, 1);
        }
        __map_region(pgdir, va_e1, va_e1 - delta, va_end - va_e1, perm, 0);
    } else {
        __map_region(pgdir, vaddr, vaddr - delta, va_end - vaddr, perm, 0);
    }
}

#define PHY_BOOT_BASE 0x40000000

void arch_map_kernel_page(addr_t max_phy_addr)
{
    if (max_phy_addr <= PHY_BOOT_BASE){
        return;
    }
    arch_map_region(NULL, kaddr(PHY_BOOT_BASE), PHY_BOOT_BASE, max_phy_addr - PHY_BOOT_BASE, PTE_W|PTE_G);
}

addr_t arch_kmap(addr_t phy, size_t sz) {
    size_t pgoff = PGOFF(phy);
    phy = PTADDR(phy);
    addr_t va = kaddr(phy);
    if(check_pgtable(va) != phy)
        arch_map_region(NULL, va, phy, sz, PTE_W | PTE_G);
    return va + pgoff;
}

void arch_kunmap(addr_t va) {
    return;
}

void arch_clear_low_page()
{
    kernel_pgd[0] = 0;
}