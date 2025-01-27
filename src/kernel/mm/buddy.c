#include <kernel/mm.h>

extern struct page *mem_pages;
void buddy_init_zone(struct zone *zone, size_t pfn_start, size_t nr_pages)
{
    zone->pfn_base = pfn_start;
    zone->nr_pages = nr_pages;
    zone->nr_free_pages = 0;
    zone->mem_map = mem_pages + pfn_start;
    for(u32 order = 0; order < MAX_ORDER; order++)
    {
        zone->free_area[order].count = 0;
        list_init(&zone->free_area[order].free_pages);
    }
}

static inline size_t __find_buddy_pfn(size_t pfn, u32 order)
{
    return (pfn ^(1L << order));
}

static inline int page_is_buddy(struct page* page, u32 order)
{
    return (page->flag & PAGE_FLAG_BUDDY) && page->order == order;
}

static inline void del_page_from_free_list(struct zone *zone, struct page *page, u32 order)
{
    list_remove(&page->buddy_free_list);
    zone->free_area[order].count--;
    page->flag &= PAGE_FLAG_BUDDY;
}

static inline void add_page_to_free_list(struct zone *zone, struct page *page, u32 order)
{
    list_init(&page->buddy_free_list);
    list_add_last(&page->buddy_free_list, &zone->free_area[order].free_pages);
    zone->free_area[order].count++;
}

static inline void set_buddy_order(struct page *page, u32 order)
{
    page->order = order;
    page->flag |= PAGE_FLAG_BUDDY;
}

static void expand_buddy(struct zone *zone, struct page *page, u32 origin_order, u32 target_order)
{
    size_t pfn_offset = 1L << origin_order;

	while (origin_order > target_order) {
		origin_order--;
		pfn_offset >>= 1;
		add_page_to_free_list(zone, &page[pfn_offset], origin_order);
		set_buddy_order(&page[pfn_offset], origin_order);
	}
}

// buddy 核心算法
// alloc
size_t buddy_page_alloc(struct zone *zone, size_t nr_pages)
{
    if((nr_pages &(-nr_pages)) != nr_pages)
    {
        error("buddy only alloc 2^n pages:req pg:%d", nr_pages);
    }
    u32 order = 0;
    for(size_t sz = nr_pages; sz&1 == 0; sz >>= 1){
        order++;
    }

    for(u32 search_order = order; search_order < MAX_ORDER; search_order++)
    {
        if(zone->free_area[search_order].count == 0) {
            continue;
        }
        struct page *page = list_front(&zone->free_area[search_order].free_pages, struct page, buddy_free_list);
        del_page_from_free_list(zone, page, search_order);
        if(search_order != order) {
            expand_buddy(zone, page, search_order, order);
        }
        set_buddy_order(page, order);
        return page_to_pfn(zone, page);
    }
    return PFN_NOTFOUND;
}

static void __buddy_page_free(struct zone *zone, size_t pfn, u32 order)
{
    size_t buddy_pfn;
	size_t combined_pfn;
    u32 max_order = MAX_ORDER - 1;
    struct page *buddy = NULL;

continue_merging:
	while (order < max_order) {
		//查找伙伴块的页号
		buddy_pfn = __find_buddy_pfn(pfn, order);
		buddy = pfn_to_page(zone, buddy_pfn);
        //查找伙伴块是否在buddy系统中
		//若不在，执行done_merging
		if (!page_is_buddy(buddy, order))
			goto done_merging;
		//若在，将伙伴块从free_list中删除
		del_page_from_free_list(zone, buddy, order);
		//合并伙伴块
		combined_pfn = buddy_pfn & pfn;     // find head of block by & 
		pfn = combined_pfn;
		order++;
	}
done_merging:
	//设置当前page的order
    struct page *page = pfn_to_page(zone, pfn);
	//将该page加入对应order的free_list中
    set_buddy_order(page, order);
	add_page_to_free_list(zone, page, order);

    return 0;
}

void buddy_page_free(struct zone *zone, size_t pfn, size_t nr_pages)
{
    if(!(pfn >= zone->pfn_base && pfn + nr_pages <= zone->pfn_base + zone->nr_pages)){
        error("pfn out of zone range:pfn_%x:%x", pfn, nr_pages);
    }
    while(nr_pages)
    {
        int order;
        size_t order_size;
        for(order = MAX_ORDER - 1; order >= 0; order--)
        {
            order_size = 1 << order;
            if(pfn % order_size == 0 && nr_pages >= order_size)
            {
                __buddy_page_free(zone, pfn, order);
                pfn += order_size;
                nr_pages -= order_size;
            }
        }
    }
}