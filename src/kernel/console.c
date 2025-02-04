#include <lib/stdio.h>
#include <drivers/serial.h>
#include <drivers/video.h>
#include <kernel/kernel.h>
#include <kernel/console.h>

#define CON_SERIAL  1
#define CON_SCREEN  2
#define CON_FILE    4

console_t console;
u32 console_flag;

void kputchar(int ch)
{
    if(console_flag & CON_SERIAL)serial_write_com(1, ch);
}


void cputchar(int ch)
{
    kputchar(ch);
    switch (ch)
    {
    case '\n':
        console.x = 0;
        console.y++;
        break;
    default:
        screen_input_char(console.x * console.xstep, console.y * console.ystep, RGB_WHITE, RGB_BLACK, ch);
        console.x ++;
        break;
    }
    
    if (console.x == console.xlimit){
        console.x = 0;
        console.y++;
    }
    if (console.y == console.ylimit)
    {
        // scroll screen
        screen_move(-console.ystep, RGB_BLACK);
        console.y--;
    }
}

void console_init()
{
    console.x = 0;
    console.y = 0;
    if (serial_init()){console_flag |= CON_SERIAL;}
    if (screen_init()){
        console_flag |= CON_SCREEN;
        console.xlimit = screen_info.width/screen_info.font_width;
        console.ylimit = screen_info.height/screen_info.font_height;
        console.xstep = screen_info.font_width;
        console.ystep = screen_info.font_height;
        info("screen: %dx%d, fb=0x%lx console:%dx%d\n", screen_info.width, screen_info.height, 
            screen_info.base, console.xlimit, console.ylimit);
    }
}

static void cputch(int ch, int *count)
{
    cputchar(ch);
    (*count)++;
}

static void kputch(int ch, int *count)
{
    kputchar(ch);
    (*count)++;
}

int vcprintf(const char *fmt, va_list ap)
{
    int cnt = 0;
    vprintfmt(cputch, &cnt, fmt, ap);
    return cnt;
}

int cprintf(const char *fmt, ...)
{
    int cnt;
    va_list ap;
	va_start(ap, fmt);
    cnt = vcprintf(fmt, ap);
	va_end(ap);
    return cnt;
}

int vkprintf(const char *fmt, va_list ap)
{
    int cnt = 0;
    vprintfmt(kputch, &cnt, fmt, ap);
    return cnt;
}

int kprintf(const char *fmt, ...)
{
    int cnt = 0;
    va_list ap;
	va_start(ap, fmt);
    cnt = vkprintf(fmt, ap);
	va_end(ap);
    return cnt;
}