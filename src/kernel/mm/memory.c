#include <kernel/kernel.h>
#include <kernel/mm.h>
#include <kernel/multiboot2.h>
#include <lib/string.h>
#include <lib/bitmap.h>

struct zone mem_zone;

struct bitmap mem_map;
struct page *mem_pages;
static addr_t reserved_end;
static int reserved_mem_ready;
static int buddy_mem_ready;
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
    info("reserved end:%lx ", reserved_end);
    reserved_mem_ready = 1;
}

// early alloc, always align page
void * reserved_mem_alloc(size_t size)
{
    void *ptr = reserved_end;
    reserved_end = align(reserved_end + size, PGSIZE);
    return ptr;
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

void pmm_init()
{
    addr_t phy_addr_limit = 0;
    struct multiboot_tag *tag = mboot_get_mboot_info(MULTIBOOT_TAG_TYPE_MMAP);
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
    info("phy limit: %lx ", phy_addr_limit);
    reserved_mem_allocator_init();
    size_t pfn_max = div_round_up(phy_addr_limit, PGSIZE);
    size_t bitmap_size = div_round_up(pfn_max, 8);
    // alloc bitmap and set all allocated
    u8* bitmap_arr = kaddr(reserved_mem_alloc(bitmap_size));
    bitmap_init(&mem_map, bitmap_arr, pfn_max);
    bitmap_set(&mem_map, MEM_PAGE_USED, 0, pfn_max);
    // alloc page stuct using reserved_mem_alloc
    mem_pages = kaddr(reserved_mem_alloc(pfn_max * sizeof(struct page)));
    buddy_init_zone(&mem_zone, 0, pfn_max);
    // second alloc, set available mem as free mem
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
            // info("add pmm range:0x%lx:%x\n", base, len);
            mm_phy_page_free(base, len / PGSIZE);   
        }
    }
    buddy_mem_ready = 1;
    info("phy mem init: %d/%d pages\n", mem_zone.nr_free_pages, mem_zone.nr_pages);
}

void mm_init(){
    // phy mem init(buddy system)
    pmm_init();
}

addr_t mm_phy_page_alloc(size_t nr_pages)
{
    if(buddy_mem_ready){
        size_t pfn =  buddy_page_alloc(&mem_zone, nr_pages);
        mem_zone.nr_free_pages -= nr_pages;
        bitmap_set(&mem_map, MEM_PAGE_USED, pfn, nr_pages);
        return (pfn << PGSHIFT);
    }else{
        if(!reserved_mem_ready){
            reserved_mem_allocator_init();
        }
        return reserved_mem_alloc(nr_pages * PGSIZE);
    }
}

void mm_phy_page_free(addr_t addr, size_t nr_pages)
{
    if(PGOFF(addr)){
        error("phy addr free should in page boundary:%lx", addr);
    }
    size_t pfn_start = addr >> PGSHIFT;
    // info("try to free pfn0x%x:%x\n",pfn_start, nr_pages);
    int check = bitmap_find(&mem_map, MEM_PAGE_AVAIL, pfn_start, nr_pages);
    if(check != -1)
    {
        warning("phy addr double free:pfn_0x%x\n", check);
    }
    mem_zone.nr_free_pages += nr_pages;
    bitmap_set(&mem_map, MEM_PAGE_AVAIL, pfn_start, nr_pages);
    buddy_page_free(&mem_zone, pfn_start, nr_pages);
}