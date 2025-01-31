#ifndef VIDEO_H
#define VIDEO_H

#include <kernel/kernel.h>

enum {
    SCREEN_VGA,
    SCREEN_RGB,
    SCREEN_INDEX
};

typedef struct {
    u8 r;
    u8 g;
    u8 b;
}color_palette;

typedef struct {
    u8 pos;
    u8 size;
}color_bit;

enum{
    VGA_BLACK,
    VGA_BLUE,
    VGA_GREEN,
    VGA_CYAN,
    VGA_RED,
    VGA_MAGENTA,
    VGA_BROWN,
    VGA_LIGHT_GRAY,
    VGA_DARK_GRAY,
    VGA_LIGHT_BLUE,
    VGA_LIGHT_GREEN,
    VGA_LIGHT_CYAN,
    VGA_LIGHT_RED,
    VGA_LIGHT_MAGENTA,
    VGA_YELLOW,
    VGA_WHITE
};
#define RGB_BLACK   0x000000
#define RGB_BLUE    0x0000AA
#define RGB_GREEN   0x00AA00
#define RGB_CYAN    0x00AAAA
#define RGB_RED     0xAA0000
#define RGB_MAGENTA 0xAA00AA
#define RGB_BROWN   0xAA5500
#define RGB_LIGHT_GRAY  0xAAAAAA
#define RGB_DARK_GRAY   0x555555
#define RGB_LIGHT_BLUE  0x5555FF
#define RGB_LIGHT_GREEN 0x55FF55
#define RGB_LIGHT_CYAN  0x55FFFF
#define RGB_LIGHT_RED   0xFF5555
#define RGB_LIGHT_MAGENTA   0xFF55FF
#define RGB_YELLOW  0xFFFF55
#define RGB_WHITE   0xFFFFFF

#define PSF_FONT_MAGIC 0x864ab572


typedef struct {
    u32 magic;         /* magic bytes to identify PSF */
    u32 version;       /* zero */
    u32 headersize;    /* offset of bitmaps in file, 32 */
    u32 flags;         /* 0 if there's no unicode table */
    u32 numglyph;      /* number of glyphs */
    u32 bytesperglyph; /* size of each glyph */
    u32 height;        /* height in pixels */
    u32 width;         /* width in pixels */
} PSF_font;

typedef struct screen_info{
    addr_t base;
    u32 width;
    u32 height;
    u32 pitch; // mem bytes for one line
    u32 bytesperglyph;
    u8 bpp; // in bytes size
    u8 type;
    u8 font_width;
    u8 font_height;
    u16 *table;
    u8 *glyph;
    PSF_font* font_header;
    union{
        struct
        {
            u16 num_colors;
            color_palette* palette;
        };
        struct 
        {
            color_bit r;
            color_bit g;
            color_bit b;
        };
    };
    
}screen_info_t;

extern screen_info_t screen_info;
u32 trans_color2pixel(u32 color);
int screen_init();
void screen_move(int ystep, u32 fill_color);
void screen_input_char(u32 x, u32 y, u32 front_color, u32 back_color, u32 chr);
#endif