#include <lib/string.h>
#include <drivers/video.h>
#include <kernel/multiboot2.h>
#include <kernel/mm.h>

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

static size_t dirty_xstart;
static size_t dirty_xend;
static size_t dirty_ystart;
static size_t dirty_yend;
static int buffer_dirty;

static void screen_flush_buffer()
{   
    if(buffer_dirty){
        size_t psize = screen_info.bpp;
        for(size_t y = dirty_ystart; y < dirty_yend; y++){
            size_t offs = screen_info.pitch*y + dirty_xstart*psize;
            memcpy(screen_info.base + offs, screen_info.buffer + offs, (dirty_xend - dirty_xstart)*psize);
        }
        buffer_dirty = 0;
    }
}

static void screen_buffer_write(size_t xstart, size_t xend, size_t ystart, size_t yend)
{
    if(buffer_dirty){
        dirty_xstart = min(dirty_xstart, xstart);
        dirty_xend = max(dirty_xend, xend);
        dirty_ystart = min(dirty_ystart, ystart);
        dirty_yend = max(dirty_yend, yend);
    } else {
        dirty_xstart = xstart;
        dirty_xend = xend;
        dirty_ystart = ystart;
        dirty_yend = yend;
        buffer_dirty = 1;
    }
}

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

int screen_init()
{
    struct multiboot_tag_framebuffer *tag = mboot_get_mboot_info(MULTIBOOT_TAG_TYPE_FRAMEBUFFER);
    if (tag == NULL) {
        return 0;
    }
    screen_info.bpp = div_round_up(tag->common.framebuffer_bpp, 8);
    screen_info.height = tag->common.framebuffer_height;
    screen_info.width = tag->common.framebuffer_width;
    screen_info.pitch = tag->common.framebuffer_pitch;
    screen_info.base = arch_kmap(tag->common.framebuffer_addr, align(screen_info.pitch*screen_info.height, PGSIZE));
    screen_info.buffer = kaddr(mm_phy_page_zalloc(align(screen_info.pitch*screen_info.height, PGSIZE)/PGSIZE));
    // addr_t test = arch_kmap(0x4000000000UL, align(screen_info.pitch*screen_info.height, PGSIZE));
    // addr_t check_pgtable(addr_t vaddr);
    // if(check_pgtable(test) != 0x4000000000UL){
    //     error("map error");
    // }
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
    return 1;
}


static void screen_put_pixel(size_t x, size_t y, u32 pixel)
{
    if(x >= screen_info.width || y >= screen_info.height){
        return;
    }
    size_t psize = screen_info.bpp;
    addr_t pptr = screen_info.buffer + y*screen_info.pitch + x*psize;
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
        error("invalid bpp");
        break;
    }
}

void screen_move(int ystep, u32 fill_color)
{
    int l = 0, r = screen_info.width, u = 0, d = screen_info.height;
    fill_color = trans_color2pixel(fill_color);
    if(ystep >= 0)
    {
        memmove(screen_info.buffer + ystep*screen_info.pitch, screen_info.buffer, (screen_info.height - ystep)*screen_info.pitch);
        for(u32 y=0; y < ystep; y++)
        {
            for(u32 x=0; x < screen_info.width; x++)
            {
                screen_put_pixel(x, y, fill_color);
            }
        }
    }else
    {
        ystep *= -1;
        memmove(screen_info.buffer, screen_info.base + ystep*screen_info.pitch, (screen_info.height - ystep)*screen_info.pitch);
        for(u32 y=screen_info.height - ystep; y < screen_info.height; y++)
        {
            for(u32 x=0; x < screen_info.width; x++)
            {
                screen_put_pixel(x, y, fill_color);
            }
        }
    }
    screen_buffer_write(0, screen_info.width, 0, screen_info.height);
    screen_flush_buffer();
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
    if (!screen_info.base){
        return;
    }
    u32 pixel, pixel_back;
    pixel = trans_color2pixel(front_color);
    pixel_back = trans_color2pixel(back_color);
    if (screen_info.type == SCREEN_VGA) 
    {
        pixel = ((((pixel & 0xF) | ((pixel_back & 0xF) << 4))<<8) | (chr & 0xFF));
        screen_put_pixel(x, y, pixel);
        screen_buffer_write(x, x+1, y, y+1);
    } else 
    {
        if(chr >= UINT16_MAX){
            return;
        }
        if(screen_info.table){
            chr = screen_info.table[chr];
        }
        if(chr >= screen_info.font_header->numglyph){
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
        screen_buffer_write(x, x+fwidth, y, y+fheight);
    }
    screen_flush_buffer();
}