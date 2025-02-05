#include <kernel/multiboot2.h>
#include <kernel/kernel.h>

struct multiboot_tag* mboot_info;
char *mboot_info_end;

void mboot_info_init()
{
    addr_t addr = mboot_info;
    u32 size = *((unsigned *) addr);
    mboot_info_end = addr + size;
}

void mboot_info_show()
{
    struct multiboot_tag *tag;
    u32 size;
    addr_t addr = mboot_info;
    size = *((unsigned *) addr);
    info("Announced mbi addr: 0x%lx, size 0x%x\n", addr, size);
    for (tag = (struct multiboot_tag *) (addr + 8);
       tag->type != MULTIBOOT_TAG_TYPE_END;
       tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag 
                                       + ((tag->size + 7) & ~7)))
    {
        switch (tag->type)
            {
            case MULTIBOOT_TAG_TYPE_CMDLINE:
            info("Command line = %s\n",
                    ((struct multiboot_tag_string *) tag)->string);
            break;
            case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
            break;
            case MULTIBOOT_TAG_TYPE_MODULE:
            break;
            case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
            break;
            case MULTIBOOT_TAG_TYPE_BOOTDEV:
            break;
            case MULTIBOOT_TAG_TYPE_MMAP:
            {
                multiboot_memory_map_t *mmap;

                debug("mmap\n");
        
                for (mmap = ((struct multiboot_tag_mmap *) tag)->entries;
                    (u8 *) mmap < (u8 *) tag + tag->size;
                    mmap = (multiboot_memory_map_t *) 
                    ((unsigned long) mmap + ((struct multiboot_tag_mmap *) tag)->entry_size))
                {
                    debug(" base_addr = 0x%llx,"
                        " length = 0x%llx, type = 0x%x\n",
                        mmap->addr, mmap->len,
                        (unsigned) mmap->type);
                }
            }
            break;
            case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
            {
                break;
            }
            default:
                break;
            }
        }
    tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag 
                                    + ((tag->size + 7) & ~7));
}

struct multiboot_tag * mboot_get_mboot_info(u32 type)
{
    addr_t addr = mboot_info;
    struct multiboot_tag *tag;
    for (tag = (struct multiboot_tag *) (addr + 8);
       tag->type != MULTIBOOT_TAG_TYPE_END;
       tag = (struct multiboot_tag *) ((u8 *) tag 
                                       + ((tag->size + 7) & ~7)))
    {
        if(tag->type == type){
            return tag;
        }
    }
    return NULL;
}