#pragma once
#include <lib/types.h>
struct bitmap{
    u8 *map;
    // bit size(8*sizeof(map))
    size_t size;
};

void bitmap_init(struct bitmap *map, u8* arr, size_t size);
void bitmap_set(struct bitmap *map, int value, size_t start, size_t len);
int bitmap_find(struct bitmap *map, int value, size_t start, size_t len);