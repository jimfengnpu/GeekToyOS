#ifndef MM_H
#define MM_H
#include <mm.h>
#include <lib/list.h>
#include <kernel/kernel.h>
#include <kernel/spinlock.h>

#define MAX_ORDER   11
#define MAX_ORDER_NR_PAGES (1 << (MAX_ORDER - 1))

#define SLAB_CACHE_MAX_NAME 32
#define SLAB_MIN_SIZE       8
#define SLAB_MAX_OBJS_PER_PAGE 512

struct slab {
    list_head list;      // 链表节点
    void *freelist;             // 空闲对象链表
    u32 inuse;         // 已用对象计数
    u32 free;          // 空闲对象计数
    u8 *obj_map;     // 对象状态位图
    struct page *page;          // 关联的物理页
};

// phy mem struct:

struct kmem_cache {
    char name[SLAB_CACHE_MAX_NAME]; // 缓存名称
    unsigned int obj_size;      // 对象大小
    unsigned int size;          // 对齐后的实际大小
    unsigned int align;         // 对齐要求
    unsigned int objs_per_page; // 每页对象数
    
    // 链表管理
    struct list_node slabs_full;    // 满slab列表
    struct list_node slabs_partial; // 部分空闲slab列表
    struct list_node slabs_free;     // 空slab列表
    
    // 统计信息
    unsigned long active_objs;
    unsigned long total_objs;

    // 锁
    struct spinlock lock;
};

// flag: 0...000B

#define PAGE_FLAG_BUDDY 1
#define PAGE_FLAG_SLAB  2

struct page{
    u32 flag;
    // buddy
    u32 order;
    struct list_node buddy_free_list;
    union 
    {
        // slab
        struct kmem_cache *slab_cache;

        // kmalloc large
        size_t nr_pages;
    }u;
    
    

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

struct task_mm {
    pgd_t *pgd;
};

// note: basic kernel page finished
void mem_init();
// init buddy 
void mm_init();
// return phy mem addr
addr_t mm_phy_page_alloc(size_t nr_pages);
addr_t mm_phy_page_zalloc(size_t nr_pages);
void mm_phy_page_free(addr_t addr, size_t nr_pages);

// kmem
void kmem_init();
struct kmem_cache *kmem_cache_create(const char *name, size_t size);
void *kmem_cache_alloc(struct kmem_cache *cachep);
void kmem_cache_free(struct kmem_cache *cachep, void *obj);

//kmalloc/kfree
void *kmalloc(size_t size);
void kfree(void * obj);
#endif