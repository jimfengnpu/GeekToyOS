#include <kernel/kernel.h>
#include <kernel/mm.h>
#include <lib/string.h>
#include <lib/stdio.h>

extern struct zone mem_zone;

struct kmem_cache mm_slab_cache;

#define NR_KMALLOC_CACHE 11
#define MAX_KMALLOC_CACHE_SIZE   2048
struct kmem_cache* mm_kmalloc_caches[NR_KMALLOC_CACHE];

struct kmalloc_info {
    const char* name;
    size_t size;
};

static struct kmalloc_info kmalloc_infos[NR_KMALLOC_CACHE] = {
    {"kmalloc-8", 8},
    {"kmalloc-16", 16},
    {"kmalloc-32", 32},
    {"kmalloc-64", 64},
    {"kmalloc-96", 96},
    {"kmalloc-128", 128},
    {"kmalloc-192", 192},
    {"kmalloc-256", 256},
    {"kmalloc-512", 512},
    {"kmalloc-1k", 1024},
    {"kmalloc-2k", 2048}
};

static void kmem_init_cache(struct kmem_cache *cachep, const char *name, size_t size, size_t align_size)
{
    // 对齐处理
    if (align_size < SLAB_MIN_SIZE)
        align_size = SLAB_MIN_SIZE;
    size = align(size, align_size);

    // 计算每页对象数
    size_t objs_per_page = (PGSIZE - sizeof(struct slab)) / size;
    objs_per_page = min(objs_per_page, SLAB_MAX_OBJS_PER_PAGE);

    strncpy(cachep->name, name, SLAB_CACHE_MAX_NAME-1);
    cachep->obj_size = size;
    cachep->size = size;
    cachep->align = align_size;
    cachep->objs_per_page = objs_per_page;
    
    // 初始化链表
    list_init(&cachep->slabs_full);
    list_init(&cachep->slabs_partial);
    list_init(&cachep->slabs_free);
}

static struct slab *cache_grow(struct kmem_cache *cachep)
{
    // 从Buddy分配页面
    addr_t phy = mm_phy_page_zalloc(1);
    if (!phy)
        return NULL;
    struct page *page = pfn_to_page(&mem_zone,  phy >> PGSHIFT);
    page->flag |= PAGE_FLAG_SLAB;
    page->u.slab_cache = cachep;

    // 初始化slab结构
    struct slab *slab = kaddr(phy);
    slab->page = page;
    slab->inuse = 0;
    slab->free = cachep->objs_per_page;
    
    // 构建空闲链表
    size_t size = cachep->size;
    char *obj = (char *)slab + PGSIZE;
    void **freelist = NULL;
    for(int i = 0; i < cachep->objs_per_page; i++)
    {
        obj -= size;
        *((void**)obj) = freelist;
        freelist = obj;
    }
    return slab;
}

void *kmem_cache_alloc(struct kmem_cache *cachep)
{
    struct slab *slab;
    
    acquire(&cachep->lock);
    
    // 尝试从partial列表获取
    if (!list_empty(&cachep->slabs_partial)) {
        slab = list_front(&cachep->slabs_partial, struct slab, list);
    } else if (!list_empty(&cachep->slabs_free)) {
        // 从free列表获取
        slab = list_front(&cachep->slabs_free, struct slab, list);
        list_move(&slab->list, &cachep->slabs_partial);
    } else {
        // 需要分配新页
        slab = cache_grow(cachep);
        if (!slab) {
            release(&cachep->lock);
            return NULL; // 内存不足
        }
        list_add_first(&slab->list, &cachep->slabs_partial);
    }
    
    // 从slab分配对象
    void *obj = slab->freelist;
    slab->freelist = *(void **)obj;
    slab->inuse++;
    slab->free--;
    
    // 更新slab状态
    if (slab->free == 0)
        list_move(&slab->list, &cachep->slabs_full);
    
    release(&cachep->lock);

    return obj;
}

void kmem_cache_free(struct kmem_cache *cachep, void *obj)
{
    acquire(&cachep->lock);

    struct slab *slab = align_down((addr_t)obj, PGSIZE);
    *((void **)obj) = slab->freelist;
    slab->freelist = obj;
    slab->inuse--;
    slab->free++;
    if (slab->inuse == 0){
        list_move(&slab->list, &cachep->slabs_free);
    }

    release(&cachep->lock);
}

/// @brief 创建 slab 缓存
struct kmem_cache *kmem_cache_create(const char *name, size_t size)
{
    struct kmem_cache *cachep = kmem_cache_alloc(&mm_slab_cache);
    kmem_init_cache(cachep, name, size, word_size);
    return cachep;
}

// 1 <= size <= MAX_KMALLOC_CACHE_SIZE, 
static int kmalloc_size_index(size_t size)
{
    int fls = 1,index;
    int last2bit;
    size = align(size, word_size) - 1;
    // bin search [0-15]+fls
    if (size & 0xFF00) {
        fls += 8;
        size >>= 8;
    }
    if (size & 0xF0) {
        fls += 4;
        size >>= 4;
    }
    if (size & 0xC) {
        fls += 2;
        size >>= 2;
    }
    last2bit = fls;
    if (size & 0x2) {
        fls ++;
        size >>= 1;
    }
    index = fls - 3;
    if ((fls == 6 || fls == 7) && (last2bit&1)) {
        index++;
    }
    if (fls == 7){
        index++;
    }else if(fls > 7){
        index += 2;
    }
    return index;
}

void *kmalloc(size_t size)
{
    if(!size){
        klog("kmalloc: kmalloc size zero\n");
        return NULL;
    }else if(size > MAX_KMALLOC_CACHE_SIZE){
        size_t num_page = align(size, PGSIZE) >> PGSHIFT;
        addr_t phy = mm_phy_page_alloc(num_page);
        struct page *page = pfn_to_page(&mem_zone, phy >> PGSHIFT);
        page->u.nr_pages = num_page;
        return kaddr(phy);
    }
    struct kmem_cache *cachep = mm_kmalloc_caches[kmalloc_size_index(size)];
    return kmem_cache_alloc(cachep);
}

void kfree(void * obj)
{
    addr_t pfn = (paddr(obj) >> PGSHIFT);
    struct page *page = pfn_to_page(&mem_zone, pfn);
    if (page->flag & PAGE_FLAG_SLAB){
        struct kmem_cache* cachep = page->u.slab_cache;
        kmem_cache_free(cachep, obj);
    } else {
        if(((addr_t)obj) & 0xFFF) {
            error("large kmalloc not page aligned\n");
        }
        mm_phy_page_free(paddr(obj), page->u.nr_pages);
    }
}

void kmem_init()
{
    char kmalloc_name[16];
    kmem_init_cache(&mm_slab_cache, "kmem_cache", sizeof(struct kmem_cache), word_size);
    for(int i = 0; i < NR_KMALLOC_CACHE; i++){
        mm_kmalloc_caches[i] = kmem_cache_create(kmalloc_infos[i].name, kmalloc_infos[i].size);
    }
}