#include <kernel/kernel.h>
#include <kernel/mm.h>
#include <kernel/multiboot2.h>
#include <lib/string.h>
#include <lib/bitmap.h>

struct zone mem_zone;

struct bitmap mem_map;
struct page *mem_pages;
static addr_t reserved_end;
static size_t pfn_max;
static addr_t phy_addr_limit;
static int reserved_mem_ready;
static int buddy_mem_ready;
static int mm_ready;
// internal pre allocator

#define MEM_PAGE_AVAIL  0
#define MEM_PAGE_USED   1

void reserved_mem_allocator_init()
{
    if(reserved_mem_ready){
        return;
    }
    reserved_end = kern_end;
    if(paddr(mboot_info_end) > reserved_end){
        reserved_end = paddr(mboot_info_end);
    }
    reserved_end = align(reserved_end, PGSIZE);
    reserved_mem_ready = 1;
}

// early alloc, always align page
void * reserved_mem_alloc(size_t size)
{
    void *ptr = reserved_end;
    reserved_end = align(reserved_end + size, PGSIZE);
    return ptr;
}

// simple page alloc using bitmap, no free
size_t block_page_alloc(size_t nr_pages)
{
    ssize_t l = 0, r = 0;
    while(r < pfn_max)
    {
        l = bitmap_find(&mem_map, MEM_PAGE_AVAIL, r, pfn_max);
        r = bitmap_find(&mem_map, MEM_PAGE_USED, l, nr_pages);
        if(r == -1) {
            return l;
        }
    }
    return PFN_NOTFOUND;
}

// mm/buddy.c
void buddy_init_zone(struct zone *zone, size_t pfn_start, size_t nr_pages);
size_t buddy_page_alloc(struct zone *zone, size_t nr_pages);
void buddy_page_free(struct zone *zone, size_t pfn, size_t nr_pages);

// general mm interface

#define for_mmap(mmap, tag)  \
for (mmap = ((struct multiboot_tag_mmap *) tag)->entries;   \
        (u8 *) mmap < (u8 *) tag + tag->size;   \
        mmap = (multiboot_memory_map_t *) ((addr_t) mmap + ((struct multiboot_tag_mmap *) tag)->entry_size))

void mem_map_init()
{
    struct multiboot_tag *tag = mboot_get_mboot_info(MULTIBOOT_TAG_TYPE_MMAP);
    if(tag == NULL){
        error("no multiboot memmap");
    }
    multiboot_memory_map_t *mmap;
    // first iter, get max phy page size
    for_mmap(mmap, tag)
    {
        if(mmap->type == MULTIBOOT_MEMORY_AVAILABLE)
        {
            phy_addr_limit = max(phy_addr_limit, mmap->addr + mmap->len);
        }
    }
    phy_addr_limit = align(phy_addr_limit, PGSIZE);
    reserved_mem_allocator_init();
    pfn_max = div_round_up(phy_addr_limit, PGSIZE);
    size_t bitmap_size = div_round_up(pfn_max, 8);
    // alloc bitmap and set all allocated
    u8* bitmap_arr = kaddr(reserved_mem_alloc(bitmap_size));
    bitmap_init(&mem_map, bitmap_arr, pfn_max);
    bitmap_set(&mem_map, MEM_PAGE_USED, 0, pfn_max);
    // second iter, set available mem as free mem
    for_mmap(mmap, tag)
    {
        if(mmap->type == MULTIBOOT_MEMORY_AVAILABLE)
        {
            addr_t base = mmap->addr;
            size_t len = mmap->len;
            if(base + len <= reserved_end){
                continue;
            }
            if(base < reserved_end){
                len = base + len - reserved_end;
                base = reserved_end;
            }
            bitmap_set(&mem_map, MEM_PAGE_AVAIL, base >> PGSHIFT, len / PGSIZE);
        }
    }
}

void buddy_init()
{
    // alloc page stuct using reserved_mem_alloc
    mem_pages = kaddr(mm_phy_page_alloc(div_round_up(pfn_max * sizeof(struct page), PGSIZE)));
    buddy_init_zone(&mem_zone, 0, pfn_max);
    ssize_t l = 0, r = 0;
    while(r < pfn_max)
    {
        l = bitmap_find(&mem_map, MEM_PAGE_AVAIL, r, pfn_max);
        r = bitmap_find(&mem_map, MEM_PAGE_USED, l, pfn_max);
        if(r == -1) {
            r = pfn_max;
        }
        buddy_page_free(&mem_zone, l, r - l);
    }
}

void pmm_init()
{ 
    info("Mem phy limit: %lx ", phy_addr_limit);
    // buddy init
    buddy_init();
    buddy_mem_ready = 1;
    info("buddy init: %d/%d pages\n", mem_zone.nr_free_pages, mem_zone.nr_pages);
}

void mem_init(){
    mem_map_init();
    mm_ready = 1;
    // now use bitmap alloc
    // map kernel page use bitmap alloc, as low page as possible
    arch_map_kernel_page(phy_addr_limit);
}

void mm_init(){
    // phy mem init(buddy system)
    pmm_init();
}

addr_t mm_phy_page_alloc(size_t nr_pages)
{
    if(!mm_ready){
        if(!reserved_mem_ready){
            reserved_mem_allocator_init();
        }
        return reserved_mem_alloc(nr_pages * PGSIZE);
    }
    size_t pfn;
    if (buddy_mem_ready) {
        pfn = buddy_page_alloc(&mem_zone, nr_pages);
    } else {
        pfn = block_page_alloc(nr_pages);
    }
    if (pfn == PFN_NOTFOUND) {
        error("alloc failed\n");
    }
    bitmap_set(&mem_map, MEM_PAGE_USED, pfn, nr_pages);
    return (pfn << PGSHIFT);
}

addr_t mm_phy_page_zalloc(size_t nr_pages)
{
    addr_t phy = mm_phy_page_alloc(nr_pages);
    memset(kaddr(phy), 0, nr_pages * PGSIZE);
    return phy;
}

void mm_phy_page_free(addr_t addr, size_t nr_pages)
{
    if(PGOFF(addr)){
        error("phy addr free should in page boundary:%lx", addr);
    }
    size_t pfn_start = addr >> PGSHIFT;
    int check = bitmap_find(&mem_map, MEM_PAGE_AVAIL, pfn_start, nr_pages);
    if(check != -1)
    {
        warning("phy addr double free:pfn_0x%x\n", check);
    }
    bitmap_set(&mem_map, MEM_PAGE_AVAIL, pfn_start, nr_pages);
    if (buddy_mem_ready){
        buddy_page_free(&mem_zone, pfn_start, nr_pages);
    }
}