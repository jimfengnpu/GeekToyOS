#include <kernel/kernel.h>
#include <kernel/mm.h>
#include <kernel/multiboot2.h>

struct zone mem_zone;

struct page *mem_map;
static addr_t reserved_end;
static int reserved_mem_ready;
static int buddy_mem_ready;
// internal pre allocator

void reserved_mem_allocator_init()
{
    if(reserved_mem_ready){
        return;
    }
    reserved_end = kaddr(kern_end);
    if(mboot_info_end > reserved_end){
        reserved_end = mboot_info_end;
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
// general mm interface

#define for_mmap(mmap, tag)  \
for (mmap = ((struct multiboot_tag_mmap *) tag)->entries;   \
        (u8 *) mmap < (u8 *) tag + tag->size;   \
        mmap = (multiboot_memory_map_t *) ((addr_t) mmap + ((struct multiboot_tag_mmap *) tag)->entry_size))

void mm_init()
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
    // alloc page stuct using reserved_mem_alloc
    reserved_mem_allocator_init();
    mem_map =  reserved_mem_alloc((phy_addr_limit / PGSIZE) * sizeof(struct page));

    

}

addr_t buddy_page_alloc(struct zone *zone, u32 nr_pages);

addr_t mm_page_alloc(u32 nr_pages)
{
    if(buddy_mem_ready){
        return buddy_page_alloc(&mem_zone, nr_pages);
    }else{
        if(!reserved_mem_ready){
            reserved_mem_allocator_init();
        }
        return reserved_mem_alloc(nr_pages * PGSIZE);
    }
}