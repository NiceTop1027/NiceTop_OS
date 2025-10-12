#include "kernel.h"
#include "vga.h"
#include "serial.h"
#include "gdt.h"
#include "idt.h"
#include "irq.h"
#include "timer.h"
#include "keyboard.h"
#include "multiboot2.h"
#include "framebuffer.h"

void kernel_main(uint32_t magic, void* multiboot_info) {

    // Initialize serial for debugging
    serial_init();
    serial_write("NiceTop OS: Serial initialized\n");

    // Initialize VGA text mode
    serial_write("NiceTop OS: Initializing VGA...\n");
    vga_init();
    vga_clear();
    serial_write("NiceTop OS: VGA initialized\n");

    // Initialize GDT
    serial_write("NiceTop OS: Initializing GDT...\n");
    gdt_init();
    serial_write("NiceTop OS: GDT initialized\n");

    // Initialize IDT
    serial_write("NiceTop OS: Initializing IDT...\n");
    idt_init();
    serial_write("NiceTop OS: IDT initialized\n");

    // Initialize IRQ
    serial_write("NiceTop OS: Initializing IRQ...\n");
    irq_init();
    serial_write("NiceTop OS: IRQ initialized\n");

    // Initialize Timer (100 Hz)
    serial_write("NiceTop OS: Initializing Timer...\n");
    timer_init(100);
    serial_write("NiceTop OS: Timer initialized\n");

    // Initialize Keyboard
    serial_write("NiceTop OS: Initializing Keyboard...\n");
    keyboard_init();
    serial_write("NiceTop OS: Keyboard initialized\n");

    // Parse Multiboot2 info for framebuffer
    serial_write("NiceTop OS: Parsing Multiboot2 info...\n");
    multiboot2_parse(magic, multiboot_info);
    serial_write("NiceTop OS: Multiboot2 parsed\n");

    // Check if framebuffer is available
    framebuffer_info_t* fb = framebuffer_get_info();
    if (fb->address != 0) {
        serial_write("NiceTop OS: Switching to graphics mode!\n");
        
        // Clear screen with gradient-like dark blue
        fb_clear(RGB(15, 25, 50));
        
        // Draw modern title bar with shadow
        fb_fill_rect(0, 0, fb->width, 50, RGB(30, 90, 180));
        fb_fill_rect(0, 50, fb->width, 2, RGB(20, 70, 150));
        
        // Title text (centered)
        const char* title = "NiceTop OS - Graphics Mode";
        uint32_t title_width = 26 * 8; // 26 characters * 8 pixels
        fb_draw_string((fb->width - title_width) / 2, 17, title, COLOR_WHITE, RGB(30, 90, 180));
        
        // Draw welcome box (larger and centered)
        uint32_t box_width = 500;
        uint32_t box_height = 280;
        uint32_t box_x = (fb->width - box_width) / 2;
        uint32_t box_y = (fb->height - box_height) / 2;
        
        // Box shadow
        fb_fill_rect(box_x + 5, box_y + 5, box_width, box_height, RGB(0, 0, 0));
        
        // Main box
        fb_fill_rect(box_x, box_y, box_width, box_height, COLOR_WHITE);
        fb_draw_rect(box_x, box_y, box_width, box_height, RGB(30, 90, 180));
        fb_draw_rect(box_x + 1, box_y + 1, box_width - 2, box_height - 2, RGB(100, 150, 220));
        
        // Content (all centered)
        fb_draw_string(box_x + (box_width - 10*8) / 2, box_y + 50, "Welcome to", RGB(60, 60, 60), COLOR_WHITE);
        
        // Large title
        const char* os_name = "NiceTop OS";
        fb_draw_string(box_x + (box_width - 10*8) / 2, box_y + 90, os_name, RGB(30, 90, 180), COLOR_WHITE);
        fb_draw_string(box_x + (box_width - 10*8) / 2 + 1, box_y + 91, os_name, RGB(30, 90, 180), COLOR_WHITE); // Bold effect
        
        // Divider line
        fb_fill_rect(box_x + 50, box_y + 140, box_width - 100, 2, RGB(200, 200, 200));
        
        // Status
        fb_draw_string(box_x + (box_width - 20*8) / 2, box_y + 170, "Graphics Mode Active", RGB(0, 150, 0), COLOR_WHITE);
        
        // Info
        const char* res = "Resolution: 1024x768";
        fb_draw_string(box_x + (box_width - 20*8) / 2, box_y + 200, res, RGB(100, 100, 100), COLOR_WHITE);
        
        const char* arch = "Architecture: i386";
        fb_draw_string(box_x + (box_width - 18*8) / 2, box_y + 225, arch, RGB(100, 100, 100), COLOR_WHITE);
        
        // Footer
        fb_fill_rect(0, fb->height - 25, fb->width, 25, RGB(20, 20, 20));
        fb_draw_string(10, fb->height - 17, "Version 0.1.0-alpha", RGB(150, 150, 150), RGB(20, 20, 20));
        
        // Hang in graphics mode
        while (1) {
            cpu_halt();
        }
    }

    // Fallback to text mode if no framebuffer
    serial_write("NiceTop OS: No framebuffer, using text mode\n");

    // Display welcome message
    serial_write("NiceTop OS: Writing to VGA...\n");
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_write("========================================\n");
    serial_write("NiceTop OS: First line written\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_write("       Welcome to NiceTop OS!\n");
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_write("========================================\n\n");

    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_write("Kernel initialized successfully\n");

    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_write("[OK] ");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_write("VGA text mode active\n");

    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_write("[OK] ");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_write("GDT initialized\n");

    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_write("[OK] ");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_write("IDT initialized\n");

    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_write("[OK] ");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_write("IRQ initialized\n");

    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_write("[OK] ");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_write("Timer initialized (100 Hz)\n");

    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_write("[OK] ");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_write("Keyboard initialized\n\n");

    vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    vga_write("System Status: ");
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_write("Running\n\n");

    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_write("Architecture: i386 (32-bit)\n");
    vga_write("Version: 0.1.0-alpha\n");
    vga_write("Build: Development\n\n");

    vga_set_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK);
    vga_write("Type something and press Enter!\n\n");
    
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_write("> ");
    
    // Simple input loop
    char input_buffer[256];
    int input_pos = 0;
    
    while (1) {
        char c = keyboard_getchar();
        
        if (c == '\n') {
            // Process command
            input_buffer[input_pos] = '\0';
            vga_write("\n");
            
            if (input_pos > 0) {
                // Echo back what was typed
                vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
                vga_write("You typed: ");
                vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
                vga_write(input_buffer);
                vga_write("\n");
                
                // Simple commands
                if (input_buffer[0] == 'h' && input_buffer[1] == 'e' && 
                    input_buffer[2] == 'l' && input_buffer[3] == 'p' && input_buffer[4] == '\0') {
                    vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
                    vga_write("Available commands:\n");
                    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
                    vga_write("  help  - Show this help\n");
                    vga_write("  clear - Clear screen\n");
                    vga_write("  time  - Show uptime\n");
                } else if (input_buffer[0] == 'c' && input_buffer[1] == 'l' && 
                           input_buffer[2] == 'e' && input_buffer[3] == 'a' && 
                           input_buffer[4] == 'r' && input_buffer[5] == '\0') {
                    vga_clear();
                    vga_set_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK);
                    vga_write("NiceTop OS Shell\n\n");
                } else if (input_buffer[0] == 't' && input_buffer[1] == 'i' && 
                           input_buffer[2] == 'm' && input_buffer[3] == 'e' && input_buffer[4] == '\0') {
                    uint32_t seconds = timer_get_ticks() / 100;
                    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
                    vga_write("Uptime: ");
                    
                    // Print seconds
                    char buf[32];
                    int i = 0;
                    if (seconds == 0) {
                        buf[i++] = '0';
                    } else {
                        char temp[32];
                        int j = 0;
                        uint32_t n = seconds;
                        while (n > 0) {
                            temp[j++] = '0' + (n % 10);
                            n /= 10;
                        }
                        while (j > 0) {
                            buf[i++] = temp[--j];
                        }
                    }
                    buf[i++] = 's';
                    buf[i] = '\0';
                    
                    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
                    vga_write(buf);
                    vga_write("\n");
                } else {
                    vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
                    vga_write("Unknown command. Type 'help' for available commands.\n");
                }
            }
            
            // New prompt
            vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
            vga_write("> ");
            input_pos = 0;
        } else if (c == '\b') {
            // Backspace
            if (input_pos > 0) {
                input_pos--;
            }
        } else {
            // Add to buffer
            if (input_pos < 255) {
                input_buffer[input_pos++] = c;
            }
        }
    }
}
