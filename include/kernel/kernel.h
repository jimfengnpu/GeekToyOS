#ifndef KERNEL_H
#define KERNEL_H

#include <lib/types.h>
#include <kernel/section.h>
#include <mem_layout.h>

#define kaddr(addr) (((addr_t) (addr) < KERN_BASE)? \
    ((addr_t)(addr) + KERN_BASE) : (addr_t) (addr))
#define paddr(addr) (((addr_t) (addr) >= KERN_BASE)? \
    ((addr_t)(addr) - KERN_BASE) : (addr_t) (addr))

#define as_kaddr(x) ((typeof(x)) kaddr(x))
#define as_paddr(x) ((typeof(x)) paddr(x))


void kernel_main();


#endif