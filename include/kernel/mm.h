#ifndef MM_H
#define MM_H
#include <mm.h>
#include <lib/list.h>
#include <kernel/kernel.h>

#define MAX_ORDER   11
#define MAX_ORDER_NR_PAGES (1 << (MAX_ORDER - 1))

// phy mem struct:

struct kmem_cache {

};

// flag: 0...000B

#define PAGE_FLAG_BUDDY 1

struct page{
    u32 flag;
    // buddy
    u32 order;
    struct list_node buddy_free_list;
};

struct free_area{
    size_t count;
    list_head free_pages;
};

struct zone {
    struct page *mem_map;
    size_t pfn_base;
    size_t nr_pages;
    size_t nr_free_pages;
    struct free_area free_area[MAX_ORDER];
};
#define pfn_to_page(zone, pfn) ((zone)->mem_map + ((pfn) - ((zone)->pfn_base)))
#define page_to_pfn(zone, page) ((zone)->pfn_base + ((page) - ((zone)->mem_map)))
#define PFN_NOTFOUND    ((size_t)-1)
// virtual(linear) mem struct:

struct mm_struct {
    pgd_t *pgd;
};

// note: basic kernel page finished, init buddy using mboot info
void mm_init();
// return phy mem addr
addr_t mm_phy_page_alloc(size_t nr_pages);
#endif