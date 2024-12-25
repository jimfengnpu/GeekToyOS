#ifndef TYPES_H
#define TYPES_H
#include <stdint.h>
typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

#ifdef ARCH_64BIT
typedef u64 addr_t;
#else
typedef u32 addr_t;
#endif


#endif