#pragma once
#include <stdint.h>
#define NULL    (0)
typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

#ifdef ARCH_64BIT
typedef u64 addr_t;
#else
typedef u32 addr_t;
#endif

#pragma once
#ifdef ARCH_64BIT
typedef unsigned long size_t;
#else
typedef unsigned int size_t;
#endif

