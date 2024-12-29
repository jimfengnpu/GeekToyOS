#include <drivers/serial.h>
#include <kernel/console.h>
#include <lib/stdio.h>

void cputchar(int ch)
{
    serial_write_com(1, ch);
}

void console_init()
{
    serial_init();

}

static void cputch(int ch, int *count)
{
    cputchar(ch);
    (*count)++;
}

int cprintf(const char *fmt, ...)
{
    int cnt = 0;
    va_list ap;
	va_start(ap, fmt);
    vprintfmt(cputch, &cnt, fmt, ap);
	va_end(ap);
    return cnt;
}