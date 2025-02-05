#include <lib/bitmap.h>
#include <lib/const.h>
#include <lib/string.h>


void bitmap_init(struct bitmap *map, u8* arr, size_t size)
{
    map->map = arr;
    map->size = size;
}

void bitmap_set(struct bitmap *map, int value, size_t start, size_t len)
{
    /*      |sb            |eb
     *------***************------
       |       |sB    |eB     |
    * 
    */  
    size_t start_byte, start_bit, end_byte, end_bit, start_bound_bit, end_bound_bit;
    u8 mask;
    start_bit = min(map->size, start);
    end_bit = min(map->size, start + len);
    start_byte = (align(start_bit, 8)) >> 3;
    end_byte = end_bit >> 3;
    start_bound_bit = (start_byte << 3) - start_bit;
    end_bound_bit = end_bit - (end_byte << 3);
    if(end_byte < start_byte) // in one same byte
    {
        mask = (~bit_mask(8 - start_bound_bit)) & bit_mask(end_bound_bit);
        if(value){
            map->map[start_byte - 1] |= mask;
        }else {
            map->map[start_byte - 1] &= (~mask);
        }
        return;
    }

    if(start_bound_bit)
    {
        mask = ~bit_mask(8 - start_bound_bit);
        if(value){
            map->map[start_byte - 1] |= mask;
        }else{
            map->map[start_byte - 1] &= (~mask); 
        }
    }

    if(end_byte > start_byte){
        if(value){
            memset(map->map + start_byte, 0xFF, end_byte - start_byte);
        }else{
            memset(map->map + start_byte, 0x0, end_byte - start_byte);
        }
    }

    if(end_bound_bit)
    {
        mask = bit_mask(end_bound_bit);
        if(value){
            map->map[end_byte] |= mask;
        }else{
            map->map[end_byte] &= (~mask); 
        }
    }
}

int bitmap_find(struct bitmap *map, int value, size_t start, size_t len)
{
    size_t start_byte, start_bit, end_byte, end_bit, start_bound_bit, end_bound_bit, found_byte;
    u8 mask, byte, skip = (value)?0x0:0xFF;
    start_bit = min(map->size, start);
    end_bit = min(map->size, start + len);
    start_byte = (align(start_bit, 8)) >> 3;
    end_byte = end_bit >> 3;
    start_bound_bit = (start_byte << 3) - start_bit;
    end_bound_bit = end_bit - (end_byte << 3);
    if(end_byte < start_byte) // in one same byte
    {
        mask = (~bit_mask(8 - start_bound_bit)) & bit_mask(end_bound_bit);
        byte = map->map[start_byte - 1];
        if(value){
            byte &= mask;
        }else {
            byte |= (~mask);
        }
        if(byte != skip){
            found_byte = start_byte - 1;
            goto _found;
        }
        return -1;
    }
    if(start_bound_bit)
    {
        mask = ~bit_mask(8 - start_bound_bit);
        byte = map->map[start_byte - 1];
        if(value){
            byte &= mask;
        }else{
            byte |= (~mask); 
        }
        if(byte != skip){
            found_byte = start_byte - 1;
            goto _found;
        }
    }

    for(u8* bit=map->map + start_byte; bit < map->map +end_byte; bit++)
    {
        byte = *bit;
        if(byte != skip){
            found_byte = bit - map->map;
            goto _found;
        }
    }

    if(end_bound_bit)
    {
        mask = bit_mask(end_bound_bit);
        byte = map->map[end_byte];
        if(value){
            byte &= mask;
        }else{
            byte |= (~mask); 
        }
        if(byte != skip){
            found_byte = end_byte;
            goto _found;
        }
    }
    return -1;
_found:
    if(value){
        byte = ~byte;
    }
    size_t pos = 0;
    while(byte&1)
    {
        pos++;
        byte >>= 1;
    }
    return (found_byte << 3) + pos;
}

int bitmap_alloc(struct bitmap *map, int value, size_t sz) 
{
    
}