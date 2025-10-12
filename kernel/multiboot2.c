#include "multiboot2.h"
#include "framebuffer.h"
#include "serial.h"

void multiboot2_parse(uint32_t magic, void* mbi) {
    if (magic != MULTIBOOT2_MAGIC) {
        serial_write("Multiboot2: Invalid magic number!\n");
        return;
    }
    
    serial_write("Multiboot2: Parsing tags...\n");
    
    // Skip the first 8 bytes (total size and reserved)
    struct multiboot_tag* tag = (struct multiboot_tag*)((uint8_t*)mbi + 8);
    
    while (tag->type != MULTIBOOT_TAG_TYPE_END) {
        switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
                struct multiboot_tag_framebuffer* fb_tag = (struct multiboot_tag_framebuffer*)tag;
                
                serial_write("Multiboot2: Framebuffer found!\n");
                serial_write("  Width: ");
                // Simple number print
                uint32_t w = fb_tag->framebuffer_width;
                char buf[16];
                int i = 0;
                if (w == 0) {
                    buf[i++] = '0';
                } else {
                    char temp[16];
                    int j = 0;
                    while (w > 0) {
                        temp[j++] = '0' + (w % 10);
                        w /= 10;
                    }
                    while (j > 0) {
                        buf[i++] = temp[--j];
                    }
                }
                buf[i] = '\0';
                serial_write(buf);
                serial_write("\n");
                
                // Initialize framebuffer
                framebuffer_init(
                    (uint32_t*)(uint32_t)fb_tag->framebuffer_addr,
                    fb_tag->framebuffer_width,
                    fb_tag->framebuffer_height,
                    fb_tag->framebuffer_pitch,
                    fb_tag->framebuffer_bpp
                );
                break;
            }
            
            case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO: {
                struct multiboot_tag_basic_meminfo* mem_tag = (struct multiboot_tag_basic_meminfo*)tag;
                serial_write("Multiboot2: Memory info found\n");
                serial_write("  Lower: ");
                uint32_t lower = mem_tag->mem_lower;
                char buf[16];
                int i = 0;
                if (lower == 0) {
                    buf[i++] = '0';
                } else {
                    char temp[16];
                    int j = 0;
                    while (lower > 0) {
                        temp[j++] = '0' + (lower % 10);
                        lower /= 10;
                    }
                    while (j > 0) {
                        buf[i++] = temp[--j];
                    }
                }
                buf[i] = '\0';
                serial_write(buf);
                serial_write(" KB\n");
                break;
            }
        }
        
        // Move to next tag (align to 8 bytes)
        tag = (struct multiboot_tag*)((uint8_t*)tag + ((tag->size + 7) & ~7));
    }
    
    serial_write("Multiboot2: Parsing complete\n");
}
