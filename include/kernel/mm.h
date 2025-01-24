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

};

struct zone {
    struct page *mem_map;
    u32 pfn_base;
    u32 nr_pages;
    list_head free_area[MAX_ORDER];
};

// virtual(linear) mem struct:

struct mm_struct {
    pgd_t *pgd;
};

// note: basic kernel page finished, init buddy using mboot info
void mm_init();
// return kern virt addr
addr_t mm_page_alloc(u32 nr_pages);
#endif