#ifndef CONSOLE_H
#define CONSOLE_H
#include <lib/stdarg.h>

typedef struct {
    u32 x, y;
    u32 xlimit;
    u32 ylimit;
    u32 xstep;
    u32 ystep;
}console_t;

void console_init();
void cputchar(int ch);
int cprintf(const char *fmt, ...);
int kprintf(const char *fmt, ...);
int vcprintf(const char *fmt, va_list ap);
int vkprintf(const char *fmt, va_list ap);

#endif