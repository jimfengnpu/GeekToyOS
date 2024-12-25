#ifndef MEM_LAYOUT_H
#define MEM_LAYOUT_H

#define KERN_BASE   0xC0000000
#define KMAPPING_BASE   0xF0000000
#define USER_START  0x00400000
#define USER_END    KERN_BASE
/*
0xF000_0000 ---- 0xFFFF_FFFF KMAPPING
0xC000_0000 ---- 0xEFFF_FFFF Kernel Linear Mapping
            ----        | User Address
            ----        |
0x0000_0000 ---- 0x0040_0000 Low Mem Direct

*/
#endif