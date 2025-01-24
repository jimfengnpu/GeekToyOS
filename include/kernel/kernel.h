#ifndef KERNEL_H
#define KERNEL_H

#include <lib/types.h>
#include <lib/const.h>
#include <cpu.h>
#include <mem_layout.h>
#include <kernel/section.h>
#include <kernel/console.h>

#define kaddr(addr) ((addr_t) ((addr_t)(addr) + KERN_BASE))
#define paddr(addr) ((addr_t) ((addr_t)(addr) - KERN_BASE))

#define as_kaddr(x) ((typeof(x)) kaddr(x))
#define as_paddr(x) ((typeof(x)) paddr(x))


void kernel_main();


void _klog(const char *, int, const char *fmt, ...);
#define klog(...) _klog(__FILE__, __LINE__, __VA_ARGS__)

void _panic(const char *, int, const char *fmt, ...);
#define panic(...) _panic(__FILE__, __LINE__, __VA_ARGS__)


#endif