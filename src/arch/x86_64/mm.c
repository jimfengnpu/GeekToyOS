#include <paging.h>
#include <kernel/kernel.h>
#include <kernel/mm.h>

pgd_t *kernel_pgd;

// flag:
#define PG_ALLOC    4
#define PG_2M   1
#define PG_1G   2
static  pte_t * pgdir_walk(pgd_t *pgdir, addr_t vaddr, addr_t *paddr, int flag)
{
    u32 pgx= PGX(vaddr), pux = PUX(vaddr), pdx = PDX(vaddr), ptx = PTX(vaddr);
    u32 pde_attr = PTE_P | PTE_W;
    int alloc = flag & PG_ALLOC;
    pde_t *pde = NULL;
    if (!(pgdir[pgx] & PTE_P)) {
        if (!alloc){
            return NULL;
        }
        pgdir[pgx] = mm_phy_page_alloc(1) | pde_attr;
    }
    pde = kaddr(PTADDR(pgdir[pgx]));
    
    // if((flag & PG_1G) || (pde[pux] & PTE_PS)) {
    //     if(paddr){
    //         *paddr = ((pde[pux] & (~bit_mask(PUXSHIFT))) | (vaddr & bit_mask(PUXSHIFT)));
    //     }
    //     return &pde[pux];
    // }
    if (!(pde[pux] & PTE_P)) {
        if (!alloc){
            return NULL;
        }
        pde[pux] = mm_phy_page_alloc(1) | pde_attr;
    }
    pde = kaddr(PTADDR(pde[pux]));
    // if((flag & PG_2M) || (pde[pdx] & PTE_PS)) {
    //     if(paddr){
    //         *paddr = ((pde[pdx] & (~bit_mask(PDXSHIFT))) | (vaddr & bit_mask(PDXSHIFT)));
    //     }
    //     return &pde[pdx];
    // }
    if (!(pde[pdx] & PTE_P)) {
        if (!alloc){
            return NULL;
        }
        pde[pdx] = mm_phy_page_alloc(1) | pde_attr;
    }
    pde = kaddr(PTADDR(pde[pdx]));
    if(paddr){
        *paddr = ((pde[ptx] & (~bit_mask(PTXSHIFT))) | (vaddr & bit_mask(PTXSHIFT)));
    }
    return &pde[ptx];
}

void map_region(pgd_t *pgdir, addr_t vaddr, addr_t paddr, size_t size, int perm, int table_level)
{
    if(pgdir == NULL){
        pgdir = kernel_pgd;
    }
    u64 pg_size = PGSIZE;// 0x1000
    if(table_level == 1)pg_size = PTSIZE; // 0x20_0000
    else if(table_level == 2)pg_size = PTSIZE*NPDENTRIES; // 0x4000_0000
    addr_t vaddr_end = align(vaddr + size, pg_size);
    addr_t _vaddr = align_down(vaddr, pg_size);
    paddr = paddr - (vaddr - _vaddr);
    vaddr = _vaddr;
    int flag = PG_ALLOC|table_level;
    for(addr_t addr = vaddr; addr < vaddr_end;)
    {
        pte_t *pte = pgdir_walk(pgdir, addr, NULL, flag);
        // if(pte == NULL){
        //     halt();
        // }
        *pte = paddr | PTE_P | perm | ((table_level)?PTE_PS:0);
        
        addr += pg_size;
        paddr += pg_size;
    }
}

extern char pud[];
void map_kernel_page()
{
    // map_region(NULL, KERN_BASE, 0, 16L*512L*PTSIZE, PTE_W, 2);
    pde_t* kernel_pud = kaddr(pud);
    for(size_t i = 1; i < 512; i++)
    {
        kernel_pud[i] = (i * 0x40000000) | PTE_W | PTE_PS | PTE_P;
    }
    // halt();
}

void check_pgtable(addr_t vaddr)
{
    addr_t addr = NULL;
    pte_t *pte = pgdir_walk(kernel_pgd, vaddr, &addr, 0);
    if(pte==NULL){
        klog("%lx=> no page\n", vaddr);
    }else{
        klog("%lx=> %lx\n", vaddr, addr);
    }
}