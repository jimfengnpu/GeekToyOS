#ifndef KERNEL_H
#define KERNEL_H

#include <lib/types.h>
#include <lib/const.h>
#include <cpu.h>
#include <mem_layout.h>
#include <kernel/section.h>
#include <kernel/console.h>

#define MAX_CORES   32

#define kaddr(addr) ((addr_t) ((addr_t)(addr) + KERN_BASE))
#define paddr(addr) ((addr_t) ((addr_t)(addr) - KERN_BASE))

#define as_kaddr(x) ((typeof(x)) kaddr(x))
#define as_paddr(x) ((typeof(x)) paddr(x))


void kernel_main();
void ap_main();

#define __FILE_PREFIX_LEN   4 //  src/
#define __FILENAME__ (__FILE__ + __FILE_PREFIX_LEN)
void _klog(const char *, int, int, const char *fmt, ...);
#define LOG_INFO    0
#define LOG_DEBUG   1
#define LOG_WARNING 2
#define LOG_ERROR   3
#define info(...) _klog(__FILENAME__, __LINE__, LOG_INFO, __VA_ARGS__)
#define debug(...) _klog(__FILENAME__, __LINE__, LOG_DEBUG, __VA_ARGS__)
#define warning(...) _klog(__FILENAME__, __LINE__, LOG_WARNING, __VA_ARGS__)
#define error(...) _klog(__FILENAME__, __LINE__, LOG_ERROR, __VA_ARGS__)
#define klog(...) _klog(__FILENAME__, __LINE__, LOG_DEBUG, __VA_ARGS__)
void _panic(const char *, int, const char *fmt, ...);
#define panic(...) _panic(__FILENAME__, __LINE__, __VA_ARGS__)


#endif