#include <drivers/video.h>
#include <kernel/multiboot2.h>

extern char* _binary_font_psf_end;
extern char* _binary_font_psf_start;
extern u32 _binary_font_psf_size;

screen_info_t screen_info;


u16 unicode_table[UINT16_MAX];

#define make_vga_palette(color) [VGA_##color] = \
    {bit_part(RGB_##color, 16, 8), bit_part(RGB_##color, 8, 8), bit_part(RGB_##color, 0, 8)}
static color_palette vga_palette[16] = {
    make_vga_palette(BLACK),
    make_vga_palette(BLUE),
    make_vga_palette(GREEN),
    make_vga_palette(CYAN),
    make_vga_palette(RED),
    make_vga_palette(MAGENTA),
    make_vga_palette(BROWN),
    make_vga_palette(LIGHT_GRAY),
    make_vga_palette(DARK_GRAY),
    make_vga_palette(LIGHT_BLUE),
    make_vga_palette(LIGHT_GREEN),
    make_vga_palette(LIGHT_CYAN),
    make_vga_palette(LIGHT_RED),
    make_vga_palette(LIGHT_MAGENTA),
    make_vga_palette(YELLOW),
    make_vga_palette(WHITE),
};

void psf_init()
{
    u16 glyph = 0;
    /* cast the address to PSF header struct */
    PSF_font *font = (PSF_font*)&_binary_font_psf_start;
    screen_info.font_height = font->height;
    screen_info.font_width = font->width;
    screen_info.bytesperglyph = font->bytesperglyph;
    screen_info.font_header = font;
    screen_info.glyph = (u8*)((u8*)&_binary_font_psf_start + font->headersize);
    /* is there a unicode table? */
    if (font->flags) {
        return; 
    }
    screen_info.table = unicode_table;
    /* get the offset of the table */
    char *s = (char *)(screen_info.glyph +
      font->numglyph * font->bytesperglyph
    );
    while(s>_binary_font_psf_end) {
        u16 uc = (u16)((unsigned char *)s[0]);
        if(uc == 0xFF) {
            glyph++;
            s++;
            continue;
        } else if(uc & 128) {
            /* UTF-8 to unicode */
            if((uc & 32) == 0 ) {
                uc = ((s[0] & 0x1F)<<6)+(s[1] & 0x3F);
                s++;
            } else
            if((uc & 16) == 0 ) {
                uc = ((((s[0] & 0xF)<<6)+(s[1] & 0x3F))<<6)+(s[2] & 0x3F);
                s+=2;
            } else
            if((uc & 8) == 0 ) {
                uc = ((((((s[0] & 0x7)<<6)+(s[1] & 0x3F))<<6)+(s[2] & 0x3F))<<6)+(s[3] & 0x3F);
                s+=3;
            } else
                uc = 0;
        }
        /* save translation */
        unicode_table[uc] = glyph;
        s++;
    }
}

void screen_init()
{
    struct multiboot_tag_framebuffer *tag = mboot_get_mboot_info(MULTIBOOT_TAG_TYPE_FRAMEBUFFER);
    screen_info.base = kaddr(tag->common.framebuffer_addr);
    screen_info.bpp = div_round_up(tag->common.framebuffer_bpp, 8);
    screen_info.height = tag->common.framebuffer_height;
    screen_info.width = tag->common.framebuffer_width;
    screen_info.pitch = tag->common.framebuffer_pitch;
    klog("screen init, fb=0x%x\n", screen_info.base);
    switch (tag->common.framebuffer_type)
    {
    case MULTIBOOT_FRAMEBUFFER_TYPE_RGB:
        screen_info.type = SCREEN_RGB;
        screen_info.r.pos = tag->framebuffer_red_field_position;
        screen_info.r.size = tag->framebuffer_red_mask_size;
        screen_info.g.pos = tag->framebuffer_green_field_position;
        screen_info.g.size = tag->framebuffer_green_mask_size;
        screen_info.b.pos = tag->framebuffer_blue_field_position;
        screen_info.b.size = tag->framebuffer_blue_mask_size;
        break;
    case MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED:
        screen_info.type = SCREEN_INDEX;
        screen_info.num_colors = tag->framebuffer_palette_num_colors;
        screen_info.palette = tag->framebuffer_palette;
        break;
    case MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT:
        screen_info.type = SCREEN_VGA;
        screen_info.num_colors = 16;
        screen_info.palette = &vga_palette;
        break;
    default:
        break;
    }
    if(screen_info.type == SCREEN_VGA)
    {
        screen_info.font_height = 1;
        screen_info.font_width = 1;
        screen_info.table = NULL;
        screen_info.font_header = NULL;
        screen_info.glyph = NULL;
    }
    else {
    // load psf
        psf_init();
    }

}


static void screen_put_pixel(u32 x, u32 y, u32 pixel)
{
    if(x >= screen_info.width || y >= screen_info.height){
        return;
    }
    u32 psize = screen_info.bpp;
    addr_t pptr = screen_info.base + y*screen_info.pitch + x*psize;
    switch (psize)
    {
    case 1:
            *((u8*)pptr) = (u8)(pixel & 0xFF);
        break;
    case 2:
            *((u16*)pptr) = (u16)(pixel & 0xFFFF);
        break;
    case 3:
            pixel = (pixel & 0xFFFFFF) | ((*(u32*)pptr) & 0xFF000000);
    case 4:
            *((u32*)pptr) = pixel;
        break;
    default:
        klog("invalid bpp");
        break;
    }
}

// color:RGB
u32 trans_color2pixel(u32 color)
{
    u32 pixel = 0;
    u8 r = bit_part(color, 16, 8),
        g = bit_part(color, 8, 8),
        b = bit_part(color, 0, 8);
    if(screen_info.type == SCREEN_RGB)
    {
        pixel |= bit_build(r, screen_info.r.pos, screen_info.r.size);
        pixel |= bit_build(g, screen_info.g.pos, screen_info.g.size);
        pixel |= bit_build(b, screen_info.b.pos, screen_info.b.size);
    }else {
        color_palette* palette = screen_info.palette;
        u32 distance = 3*256*256, _distance;
        for(int i = 0; i < screen_info.num_colors; i++)
        {
            _distance = (r - palette[i].r)*(r - palette[i].r)
                + (g - palette[i].g)*(g - palette[i].g)
                + (b - palette[i].b)*(b - palette[i].b);
            if (_distance < distance)
            {
                pixel = i;
                distance = _distance;
            }
        }
    }
    return pixel;
}


void screen_input_char(u32 x, u32 y, u32 front_color, u32 back_color, u32 chr)
{
    u32 pixel, pixel_back;
    pixel = trans_color2pixel(front_color);
    pixel_back = trans_color2pixel(back_color);
    if (screen_info.type == SCREEN_VGA) 
    {
        pixel = ((((pixel & 0xF) | ((pixel_back & 0xF) << 4))<<8) | (chr & 0xFF));
        screen_put_pixel(x, y, pixel);
    } else 
    {
        if(chr >= UINT16_MAX){
            return;
        }
        if(screen_info.table){
            chr = screen_info.table[chr];
        }
        if(chr >= screen_info.font_header->numglyph || chr < 0){
            chr = 0;
        }
        u8 *glyph = screen_info.glyph + chr*screen_info.bytesperglyph;
        u32 g;
        int line,mask, dx, dy,
            fwidth = screen_info.font_width,
            fheight = screen_info.font_height;
        int bytesperline=div_round_up(fwidth, 8);
        for(dy=0;dy < fheight; dy++){
            mask=1<<(fwidth-1);
            g = 0;
            for(int i = 0; i < bytesperline; i++){
                g = (g << 8) + *(glyph + i);
            }
            g >>= (bytesperline*8 - fwidth);
            /* display a row */
            for(dx=0;dx < fwidth; dx++){
                screen_put_pixel(x+dx, y+dy, (g & mask)?pixel:pixel_back);
                /* adjust to the next pixel */
                mask >>= 1;
            }
            /* adjust to the next line */
            glyph += bytesperline;
        }
    }
}