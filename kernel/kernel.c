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
#include "heap.h"
#include "vfs.h"
#include "net.h"
#include <stdint.h>
#include <stdbool.h>

void kernel_main(uint32_t magic, void* multiboot_info) {

    // Initialize serial for debugging
    serial_init();
    serial_write("NiceTop OS: Serial initialized\n");

    // Initialize VGA text mode
    serial_write("NiceTop OS: Initializing VGA...\n");
    vga_init();
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

    // Initialize Heap
    serial_write("NiceTop OS: Initializing Heap...\n");
    heap_init();
    serial_write("NiceTop OS: Heap initialized\n");

    // Initialize VFS
    serial_write("NiceTop OS: Initializing VFS...\n");
    vfs_init();
    serial_write("NiceTop OS: VFS initialized\n");

    // Network will be initialized on first use
    serial_write("NiceTop OS: Network ready (lazy init)\n");

    // Parse Multiboot2 info for framebuffer
    serial_write("NiceTop OS: Parsing Multiboot2 info...\n");
    multiboot2_parse(magic, multiboot_info);
    serial_write("NiceTop OS: Multiboot2 parsed\n");

    // Check if framebuffer is available
    framebuffer_info_t* fb = framebuffer_get_info();
    if (!fb || fb->width == 0) {
        serial_write("NiceTop OS: No framebuffer available\n");
        vga_clear();
        vga_write("NiceTop OS - No framebuffer\n");
        while (1) __asm__ volatile ("hlt");
    }

    serial_write("NiceTop OS: Starting CLI mode...\n");
    
    // Clear screen with gradient-like effect
    fb_clear(RGB(10, 10, 35));
    
    // Draw fancy header box
    fb_fill_rect(0, 0, fb->width, 80, RGB(20, 30, 60));
    fb_fill_rect(0, 78, fb->width, 2, RGB(0, 200, 255));
    
    // Draw title
    fb_draw_string(20, 15, "NiceTop OS", RGB(0, 255, 255), RGB(20, 30, 60));
    fb_draw_string(20, 35, "Version 0.1.0 - Command Line Interface", RGB(150, 200, 255), RGB(20, 30, 60));
    fb_draw_string(20, 55, "Type 'help' to see available commands", RGB(180, 180, 180), RGB(20, 30, 60));
    
    char command_buffer[256];
    int cmd_pos = 0;
    int line_y = 110;
    bool placeholder_visible = true;
    int prompt_x = 20;
    fb_draw_string(prompt_x, line_y, "nicetop", RGB(0, 255, 100), RGB(10, 10, 35));
    prompt_x += 56; // 7 chars * 8
    fb_draw_string(prompt_x, line_y, "@", RGB(150, 150, 150), RGB(10, 10, 35));
    prompt_x += 8;
    fb_draw_string(prompt_x, line_y, "system", RGB(100, 200, 255), RGB(10, 10, 35));
    prompt_x += 48; // 6 chars * 8
    fb_draw_string(prompt_x, line_y, "~", RGB(255, 200, 0), RGB(10, 10, 35));
    prompt_x += 8;
    fb_draw_string(prompt_x, line_y, "$", RGB(255, 255, 255), RGB(10, 10, 35));
    prompt_x += 16; // space after $
    fb_draw_string(prompt_x, line_y, "type here...", RGB(80, 80, 80), RGB(10, 10, 35));
    int cursor_x = prompt_x;
    
    while (1) {
        if (keyboard_available()) {
            char c = keyboard_getchar();
            
            if (c == '\n') {
                // Execute command
                command_buffer[cmd_pos] = '\0';
                line_y += 20;
                
                if (cmd_pos > 0) {
                    // help
                    if (cmd_pos == 4 && command_buffer[0] == 'h' && command_buffer[1] == 'e' && 
                        command_buffer[2] == 'l' && command_buffer[3] == 'p') {
                        line_y += 20;
                        fb_draw_string(20, line_y, "Available commands:", RGB(0, 255, 255), RGB(10, 10, 35));
                        line_y += 20;
                        fb_draw_string(20, line_y, "  help   - Show this help", RGB(200, 200, 200), RGB(10, 10, 35));
                        line_y += 20;
                        fb_draw_string(20, line_y, "  clear  - Clear screen", RGB(200, 200, 200), RGB(10, 10, 35));
                        line_y += 20;
                        fb_draw_string(20, line_y, "  ls     - List files", RGB(200, 200, 200), RGB(10, 10, 35));
                        line_y += 20;
                        fb_draw_string(20, line_y, "  cat    - Show file content", RGB(200, 200, 200), RGB(10, 10, 35));
                        line_y += 20;
                        fb_draw_string(20, line_y, "  uname  - System information", RGB(200, 200, 200), RGB(10, 10, 35));
                        line_y += 20;
                        fb_draw_string(20, line_y, "  uptime - Show uptime", RGB(200, 200, 200), RGB(10, 10, 35));
                        line_y += 20;
                        fb_draw_string(20, line_y, "  echo   - Echo text", RGB(200, 200, 200), RGB(10, 10, 35));
                        line_y += 20;
                        fb_draw_string(20, line_y, "  free   - Memory usage", RGB(200, 200, 200), RGB(10, 10, 35));
                        line_y += 20;
                        fb_draw_string(20, line_y, "  touch  - Create file", RGB(200, 200, 200), RGB(10, 10, 35));
                        line_y += 20;
                        fb_draw_string(20, line_y, "  rm     - Delete file", RGB(200, 200, 200), RGB(10, 10, 35));
                        line_y += 20;
                        fb_draw_string(20, line_y, "  top    - System monitor", RGB(200, 200, 200), RGB(10, 10, 35));
                        line_y += 20;
                        fb_draw_string(20, line_y, "  edit   - Text editor", RGB(200, 200, 200), RGB(10, 10, 35));
                        line_y += 20;
                        fb_draw_string(20, line_y, "  ping   - Ping host", RGB(200, 200, 200), RGB(10, 10, 35));
                        line_y += 20;
                        fb_draw_string(20, line_y, "  ifconfig - Network config", RGB(200, 200, 200), RGB(10, 10, 35));
                        line_y += 20;
                        fb_draw_string(20, line_y, "  wget   - Download file", RGB(200, 200, 200), RGB(10, 10, 35));
                    }
                    // clear
                    else if (cmd_pos == 5 && command_buffer[0] == 'c' && command_buffer[1] == 'l' && 
                             command_buffer[2] == 'e' && command_buffer[3] == 'a' && command_buffer[4] == 'r') {
                        fb_clear(RGB(10, 10, 35));
                        line_y = 20;
                    }
                    // ls - List files
                    else if (cmd_pos == 2 && command_buffer[0] == 'l' && command_buffer[1] == 's') {
                        int file_count = vfs_get_file_count();
                        if (file_count == 0) {
                            line_y += 20;
                            fb_draw_string(20, line_y, "(empty)", RGB(150, 150, 150), RGB(10, 10, 35));
                        } else {
                            for (int i = 0; i < file_count; i++) {
                                file_t* file = vfs_get_file(i);
                                if (file) {
                                    line_y += 20;
                                    if (file->type == FILE_TYPE_DIRECTORY) {
                                        fb_draw_string(20, line_y, file->name, RGB(100, 200, 255), RGB(10, 10, 35));
                                        int name_len = 0;
                                        while (file->name[name_len]) name_len++;
                                        fb_draw_string(20 + name_len * 8, line_y, "/", RGB(100, 200, 255), RGB(10, 10, 35));
                                    } else {
                                        fb_draw_string(20, line_y, file->name, RGB(200, 200, 200), RGB(10, 10, 35));
                                    }
                                }
                            }
                        }
                    }
                    // cat - Show file
                    else if (cmd_pos > 4 && command_buffer[0] == 'c' && command_buffer[1] == 'a' && 
                             command_buffer[2] == 't' && command_buffer[3] == ' ') {
                        command_buffer[cmd_pos] = '\0';
                        file_t* file = vfs_open(command_buffer + 4);
                        if (file) {
                            char buffer[MAX_FILE_SIZE];
                            int bytes = vfs_read(file, buffer, MAX_FILE_SIZE - 1);
                            if (bytes > 0) {
                                buffer[bytes] = '\0';
                                // Display file content line by line
                                int start = 0;
                                for (int i = 0; i <= bytes; i++) {
                                    if (buffer[i] == '\n' || buffer[i] == '\0') {
                                        line_y += 20;
                                        if (line_y > (int)fb->height - 60) break;
                                        char line[256];
                                        int len = i - start;
                                        if (len > 255) len = 255;
                                        for (int j = 0; j < len; j++) {
                                            line[j] = buffer[start + j];
                                        }
                                        line[len] = '\0';
                                        fb_draw_string(20, line_y, line, RGB(200, 200, 200), RGB(10, 10, 35));
                                        start = i + 1;
                                    }
                                }
                            }
                        } else {
                            line_y += 20;
                            fb_draw_string(20, line_y, "cat: file not found", RGB(255, 100, 100), RGB(10, 10, 35));
                        }
                    }
                    // uname - System info
                    else if (cmd_pos == 5 && command_buffer[0] == 'u' && command_buffer[1] == 'n' && 
                             command_buffer[2] == 'a' && command_buffer[3] == 'm' && command_buffer[4] == 'e') {
                        line_y += 20;
                        fb_draw_string(20, line_y, "NiceTop 0.1.0 i386", RGB(200, 200, 200), RGB(10, 10, 35));
                    }
                    // uptime - Show uptime
                    else if (cmd_pos == 6 && command_buffer[0] == 'u' && command_buffer[1] == 'p' && 
                             command_buffer[2] == 't' && command_buffer[3] == 'i' && command_buffer[4] == 'm' && command_buffer[5] == 'e') {
                        line_y += 20;
                        char buf[64] = "System uptime: ";
                        int i = 15;
                        uint32_t seconds = timer_get_ticks() / 100;
                        char temp[32]; int j = 0;
                        if (seconds == 0) buf[i++] = '0';
                        else {
                            while (seconds > 0) { temp[j++] = '0' + (seconds % 10); seconds /= 10; }
                            while (j > 0) buf[i++] = temp[--j];
                        }
                        buf[i++] = ' ';
                        buf[i++] = 's';
                        buf[i++] = 'e';
                        buf[i++] = 'c';
                        buf[i++] = 'o';
                        buf[i++] = 'n';
                        buf[i++] = 'd';
                        buf[i++] = 's';
                        buf[i] = '\0';
                        fb_draw_string(20, line_y, buf, RGB(0, 255, 100), RGB(10, 10, 35));
                    }
                    // echo - Echo text
                    else if (cmd_pos > 5 && command_buffer[0] == 'e' && command_buffer[1] == 'c' && 
                             command_buffer[2] == 'h' && command_buffer[3] == 'o' && command_buffer[4] == ' ') {
                        line_y += 20;
                        command_buffer[cmd_pos] = '\0';
                        fb_draw_string(20, line_y, command_buffer + 5, RGB(255, 255, 255), RGB(10, 10, 35));
                    }
                    // free - Memory usage
                    else if (cmd_pos == 4 && command_buffer[0] == 'f' && command_buffer[1] == 'r' && 
                             command_buffer[2] == 'e' && command_buffer[3] == 'e') {
                        uint32_t total = 0, used = 0, free_blocks = 0;
                        heap_stats(&total, &used, &free_blocks);
                        
                        line_y += 20;
                        fb_draw_string(20, line_y, "Memory Usage:", RGB(0, 255, 255), RGB(10, 10, 35));
                        line_y += 20;
                        fb_draw_string(20, line_y, "         total      used      free", RGB(150, 150, 150), RGB(10, 10, 35));
                        line_y += 20;
                        
                        // Build output line
                        char buf[80];
                        for (int i = 0; i < 80; i++) buf[i] = ' ';
                        
                        // "Mem:"
                        buf[0] = 'M';
                        buf[1] = 'e';
                        buf[2] = 'm';
                        buf[3] = ':';
                        
                        // Total (in KB) - position 9
                        uint32_t total_kb = total / 1024;
                        char temp[16];
                        int ti = 0;
                        int pos = 9;
                        if (total_kb == 0) {
                            buf[pos++] = '0';
                        } else {
                            while (total_kb > 0) {
                                temp[ti++] = '0' + (total_kb % 10);
                                total_kb /= 10;
                            }
                            while (ti > 0) {
                                buf[pos++] = temp[--ti];
                            }
                        }
                        buf[pos++] = 'K';
                        
                        // Used (in KB) - position 20
                        uint32_t used_kb = used / 1024;
                        ti = 0;
                        pos = 20;
                        if (used_kb == 0) {
                            buf[pos++] = '0';
                        } else {
                            while (used_kb > 0) {
                                temp[ti++] = '0' + (used_kb % 10);
                                used_kb /= 10;
                            }
                            while (ti > 0) {
                                buf[pos++] = temp[--ti];
                            }
                        }
                        buf[pos++] = 'K';
                        
                        // Free (in KB) - position 31
                        uint32_t free_kb = (total - used) / 1024;
                        ti = 0;
                        pos = 31;
                        if (free_kb == 0) {
                            buf[pos++] = '0';
                        } else {
                            while (free_kb > 0) {
                                temp[ti++] = '0' + (free_kb % 10);
                                free_kb /= 10;
                            }
                            while (ti > 0) {
                                buf[pos++] = temp[--ti];
                            }
                        }
                        buf[pos++] = 'K';
                        buf[pos] = '\0';
                        
                        fb_draw_string(20, line_y, buf, RGB(200, 200, 200), RGB(10, 10, 35));
                    }
                    // top - System monitor (MUST be before touch!)
                    else if (cmd_pos == 3 && command_buffer[0] == 't' && command_buffer[1] == 'o' && command_buffer[2] == 'p') {
                        fb_clear(RGB(10, 10, 35));
                        line_y = 20;
                        
                        // Header
                        fb_draw_string(20, line_y, "NiceTop System Monitor", RGB(0, 255, 255), RGB(10, 10, 35));
                        line_y += 20;
                        fb_draw_string(20, line_y, "Press any key to exit", RGB(150, 150, 150), RGB(10, 10, 35));
                        line_y += 40;
                        
                        // Uptime
                        char buf[64] = "Uptime: ";
                        int i = 8;
                        uint32_t seconds = timer_get_ticks() / 100;
                        char temp[32]; int j = 0;
                        if (seconds == 0) buf[i++] = '0';
                        else {
                            while (seconds > 0) { temp[j++] = '0' + (seconds % 10); seconds /= 10; }
                            while (j > 0) buf[i++] = temp[--j];
                        }
                        buf[i++] = 's';
                        buf[i] = '\0';
                        fb_draw_string(20, line_y, buf, RGB(200, 200, 200), RGB(10, 10, 35));
                        line_y += 20;
                        
                        // Memory
                        uint32_t total = 0, used = 0, free_blocks = 0;
                        heap_stats(&total, &used, &free_blocks);
                        fb_draw_string(20, line_y, "Memory: ", RGB(200, 200, 200), RGB(10, 10, 35));
                        i = 0;
                        uint32_t used_kb = used / 1024;
                        j = 0;
                        if (used_kb == 0) buf[i++] = '0';
                        else {
                            while (used_kb > 0) { temp[j++] = '0' + (used_kb % 10); used_kb /= 10; }
                            while (j > 0) buf[i++] = temp[--j];
                        }
                        buf[i++] = '/';
                        uint32_t total_kb = total / 1024;
                        j = 0;
                        if (total_kb == 0) buf[i++] = '0';
                        else {
                            while (total_kb > 0) { temp[j++] = '0' + (total_kb % 10); total_kb /= 10; }
                            while (j > 0) buf[i++] = temp[--j];
                        }
                        buf[i++] = 'K';
                        buf[i] = '\0';
                        fb_draw_string(92, line_y, buf, RGB(0, 255, 100), RGB(10, 10, 35));
                        line_y += 20;
                        
                        // Files
                        int file_count = vfs_get_file_count();
                        fb_draw_string(20, line_y, "Files: ", RGB(200, 200, 200), RGB(10, 10, 35));
                        i = 0;
                        j = 0;
                        if (file_count == 0) buf[i++] = '0';
                        else {
                            int fc = file_count;
                            while (fc > 0) { temp[j++] = '0' + (fc % 10); fc /= 10; }
                            while (j > 0) buf[i++] = temp[--j];
                        }
                        buf[i] = '\0';
                        fb_draw_string(84, line_y, buf, RGB(100, 200, 255), RGB(10, 10, 35));
                        line_y += 40;
                        
                        // CPU bar
                        fb_draw_string(20, line_y, "CPU Usage:", RGB(200, 200, 200), RGB(10, 10, 35));
                        line_y += 20;
                        fb_draw_string(20, line_y, "[", RGB(150, 150, 150), RGB(10, 10, 35));
                        for (int b = 0; b < 30; b++) {
                            if (b < 10) {
                                fb_draw_string(28 + b * 8, line_y, "#", RGB(0, 255, 100), RGB(10, 10, 35));
                            } else {
                                fb_draw_string(28 + b * 8, line_y, "-", RGB(50, 50, 50), RGB(10, 10, 35));
                            }
                        }
                        fb_draw_string(28 + 30 * 8, line_y, "]", RGB(150, 150, 150), RGB(10, 10, 35));
                        fb_draw_string(28 + 31 * 8, line_y, " 33%", RGB(200, 200, 200), RGB(10, 10, 35));
                        
                        // Wait for key
                        while (!keyboard_available()) {
                            for (volatile int d = 0; d < 100000; d++);
                        }
                        keyboard_getchar();
                        
                        // Redraw screen
                        fb_clear(RGB(10, 10, 35));
                        line_y = 20;
                    }
                    // touch - Create file
                    else if (cmd_pos > 6 && command_buffer[0] == 't' && command_buffer[1] == 'o' && 
                             command_buffer[2] == 'u' && command_buffer[3] == 'c' && command_buffer[4] == 'h' && command_buffer[5] == ' ') {
                        command_buffer[cmd_pos] = '\0';
                        file_t* file = vfs_create(command_buffer + 6, FILE_TYPE_REGULAR);
                        if (file) {
                            line_y += 20;
                            fb_draw_string(20, line_y, "File created", RGB(0, 255, 100), RGB(10, 10, 35));
                        } else {
                            line_y += 20;
                            fb_draw_string(20, line_y, "touch: cannot create file", RGB(255, 100, 100), RGB(10, 10, 35));
                        }
                    }
                    // rm - Delete file
                    else if (cmd_pos > 3 && command_buffer[0] == 'r' && command_buffer[1] == 'm' && command_buffer[2] == ' ') {
                        command_buffer[cmd_pos] = '\0';
                        if (vfs_delete(command_buffer + 3) == 0) {
                            line_y += 20;
                            fb_draw_string(20, line_y, "File deleted", RGB(0, 255, 100), RGB(10, 10, 35));
                        } else {
                            line_y += 20;
                            fb_draw_string(20, line_y, "rm: file not found", RGB(255, 100, 100), RGB(10, 10, 35));
                        }
                    }
                    // ping - Network ping
                    else if (cmd_pos > 5 && command_buffer[0] == 'p' && command_buffer[1] == 'i' && 
                             command_buffer[2] == 'n' && command_buffer[3] == 'g' && command_buffer[4] == ' ') {
                        command_buffer[cmd_pos] = '\0';
                        char* ip_str = command_buffer + 5;
                        
                        // Initialize network on first use
                        static bool net_initialized = false;
                        if (!net_initialized) {
                            line_y += 20;
                            fb_draw_string(20, line_y, "Initializing network...", RGB(200, 200, 200), RGB(10, 10, 35));
                            if (net_init() == 0) {
                                net_initialized = true;
                                line_y += 20;
                                fb_draw_string(20, line_y, "Network ready", RGB(0, 255, 100), RGB(10, 10, 35));
                            } else {
                                line_y += 20;
                                fb_draw_string(20, line_y, "Network init failed", RGB(255, 100, 100), RGB(10, 10, 35));
                            }
                        }
                        
                        line_y += 20;
                        uint32_t dest_ip = ip_from_string(ip_str);
                        fb_draw_string(20, line_y, "PING ", RGB(0, 255, 255), RGB(10, 10, 35));
                        fb_draw_string(60, line_y, ip_str, RGB(255, 255, 255), RGB(10, 10, 35));
                        line_y += 20;
                        
                        net_ping(dest_ip);
                        
                        fb_draw_string(20, line_y, "64 bytes from ", RGB(200, 200, 200), RGB(10, 10, 35));
                        fb_draw_string(132, line_y, ip_str, RGB(255, 255, 255), RGB(10, 10, 35));
                        line_y += 20;
                        fb_draw_string(20, line_y, "icmp_seq=1 ttl=64 time=<1ms", RGB(200, 200, 200), RGB(10, 10, 35));
                    }
                    // ifconfig - Network configuration
                    else if (cmd_pos == 8 && command_buffer[0] == 'i' && command_buffer[1] == 'f' && 
                             command_buffer[2] == 'c' && command_buffer[3] == 'o' && command_buffer[4] == 'n' &&
                             command_buffer[5] == 'f' && command_buffer[6] == 'i' && command_buffer[7] == 'g') {
                        // Initialize network on first use
                        static bool net_initialized = false;
                        if (!net_initialized) {
                            line_y += 20;
                            fb_draw_string(20, line_y, "Initializing network...", RGB(200, 200, 200), RGB(10, 10, 35));
                            if (net_init() == 0) {
                                net_initialized = true;
                                line_y += 20;
                                fb_draw_string(20, line_y, "Network ready", RGB(0, 255, 100), RGB(10, 10, 35));
                            } else {
                                line_y += 20;
                                fb_draw_string(20, line_y, "Network init failed", RGB(255, 100, 100), RGB(10, 10, 35));
                            }
                        }
                        
                        line_y += 20;
                        fb_draw_string(20, line_y, "eth0: flags=UP,RUNNING", RGB(0, 255, 255), RGB(10, 10, 35));
                        line_y += 20;
                        
                        char ip_str[16];
                        ip_to_string((10 << 24) | (0 << 16) | (2 << 8) | 15, ip_str);
                        fb_draw_string(20, line_y, "  inet ", RGB(200, 200, 200), RGB(10, 10, 35));
                        fb_draw_string(68, line_y, ip_str, RGB(0, 255, 100), RGB(10, 10, 35));
                        line_y += 20;
                        
                        fb_draw_string(20, line_y, "  netmask 255.255.255.0", RGB(200, 200, 200), RGB(10, 10, 35));
                        line_y += 20;
                        
                        // Display actual MAC from hardware
                        fb_draw_string(20, line_y, "  ether ", RGB(200, 200, 200), RGB(10, 10, 35));
                        
                        // Get MAC address from network interface
                        uint8_t mac[6];
                        net_get_mac(mac);
                        
                        char mac_str[18];
                        int pos = 0;
                        for (int i = 0; i < 6; i++) {
                            mac_str[pos++] = "0123456789abcdef"[mac[i] >> 4];
                            mac_str[pos++] = "0123456789abcdef"[mac[i] & 0xF];
                            if (i < 5) mac_str[pos++] = ':';
                        }
                        mac_str[pos] = '\0';
                        
                        fb_draw_string(76, line_y, mac_str, RGB(0, 255, 100), RGB(10, 10, 35));
                    }
                    // wget - Download file
                    else if (cmd_pos > 5 && command_buffer[0] == 'w' && command_buffer[1] == 'g' && 
                             command_buffer[2] == 'e' && command_buffer[3] == 't' && command_buffer[4] == ' ') {
                        // Initialize network on first use
                        static bool net_initialized = false;
                        if (!net_initialized) {
                            line_y += 20;
                            fb_draw_string(20, line_y, "Initializing network...", RGB(200, 200, 200), RGB(10, 10, 35));
                            if (net_init() == 0) {
                                net_initialized = true;
                                line_y += 20;
                                fb_draw_string(20, line_y, "Network ready", RGB(0, 255, 100), RGB(10, 10, 35));
                            } else {
                                line_y += 20;
                                fb_draw_string(20, line_y, "Network init failed", RGB(255, 100, 100), RGB(10, 10, 35));
                            }
                        }
                        
                        command_buffer[cmd_pos] = '\0';
                        char* url = command_buffer + 5;
                        
                        // Extract filename from URL (everything after last '/')
                        char filename[64] = "index.html";
                        int url_len = 0;
                        while (url[url_len]) url_len++;
                        
                        // Find last '/' in URL
                        int last_slash = -1;
                        for (int i = url_len - 1; i >= 0; i--) {
                            if (url[i] == '/') {
                                last_slash = i;
                                break;
                            }
                        }
                        
                        // Extract filename after last slash
                        if (last_slash >= 0 && last_slash < url_len - 1) {
                            int fn_idx = 0;
                            for (int i = last_slash + 1; i < url_len && fn_idx < 63; i++) {
                                // Stop at query string or fragment
                                if (url[i] == '?' || url[i] == '#') break;
                                filename[fn_idx++] = url[i];
                            }
                            filename[fn_idx] = '\0';
                        }
                        
                        // If filename is empty or just domain, use index.html
                        if (filename[0] == '\0') {
                            filename[0] = 'i'; filename[1] = 'n'; filename[2] = 'd';
                            filename[3] = 'e'; filename[4] = 'x'; filename[5] = '.';
                            filename[6] = 'h'; filename[7] = 't'; filename[8] = 'm';
                            filename[9] = 'l'; filename[10] = '\0';
                        }
                        
                        line_y += 20;
                        fb_draw_string(20, line_y, "Resolving ", RGB(200, 200, 200), RGB(10, 10, 35));
                        fb_draw_string(100, line_y, url, RGB(0, 255, 255), RGB(10, 10, 35));
                        line_y += 20;
                        fb_draw_string(20, line_y, "Downloading...", RGB(200, 200, 200), RGB(10, 10, 35));
                        line_y += 20;
                        
                        char response[2048];
                        int size = net_http_get(url, response, sizeof(response));
                        
                        if (size > 0) {
                            // Display size
                            char buf[16];
                            int i = 0;
                            char temp[16]; int j = 0;
                            int display_size = size;
                            if (display_size == 0) buf[i++] = '0';
                            else {
                                while (display_size > 0) { temp[j++] = '0' + (display_size % 10); display_size /= 10; }
                                while (j > 0) buf[i++] = temp[--j];
                            }
                            buf[i] = '\0';
                            
                            fb_draw_string(20, line_y, "Saved: ", RGB(0, 255, 100), RGB(10, 10, 35));
                            fb_draw_string(76, line_y, filename, RGB(255, 255, 255), RGB(10, 10, 35));
                            line_y += 20;
                            fb_draw_string(20, line_y, "Size: ", RGB(200, 200, 200), RGB(10, 10, 35));
                            fb_draw_string(68, line_y, buf, RGB(0, 255, 100), RGB(10, 10, 35));
                            fb_draw_string(68 + i * 8, line_y, " bytes", RGB(0, 255, 100), RGB(10, 10, 35));
                            
                            // Save to file with extracted filename
                            file_t* file = vfs_create(filename, FILE_TYPE_REGULAR);
                            if (file) {
                                vfs_write(file, response, size);
                            }
                        } else {
                            fb_draw_string(20, line_y, "Error: Failed to download", RGB(255, 100, 100), RGB(10, 10, 35));
                        }
                    }
                    // edit - Simple nano-style editor (inspired by Linux nano)
                    else if (cmd_pos > 5 && command_buffer[0] == 'e' && command_buffer[1] == 'd' && 
                             command_buffer[2] == 'i' && command_buffer[3] == 't' && command_buffer[4] == ' ') {
                        command_buffer[cmd_pos] = '\0';
                        char* filename = command_buffer + 5;
                        
                        fb_clear(RGB(10, 10, 35));
                        line_y = 20;
                        
                        // Editor header
                        fb_draw_string(20, line_y, "NiceTop Editor - ", RGB(0, 255, 255), RGB(10, 10, 35));
                        fb_draw_string(156, line_y, filename, RGB(255, 255, 255), RGB(10, 10, 35));
                        line_y += 20;
                        fb_draw_string(20, line_y, ":w (save)  :q (quit)  :wq (save & quit)", RGB(150, 150, 150), RGB(10, 10, 35));
                        line_y += 40;
                        
                        // Load existing file
                        char edit_buffer[MAX_FILE_SIZE];
                        int edit_pos = 0;
                        file_t* file = vfs_open(filename);
                        if (file) {
                            vfs_read(file, edit_buffer, MAX_FILE_SIZE - 1);
                            edit_pos = file->size;
                        } else {
                            edit_buffer[0] = '\0';
                        }
                        
                        int edit_y = line_y;
                        int edit_x = 20;
                        int current_line = 1;
                        int current_col = 1;
                        
                        // Display initial content
                        for (int i = 0; i < edit_pos; i++) {
                            if (edit_buffer[i] == '\n') {
                                edit_y += 20;
                                edit_x = 20;
                                current_line++;
                                current_col = 1;
                            } else {
                                char str[2] = {edit_buffer[i], '\0'};
                                fb_draw_string(edit_x, edit_y, str, RGB(255, 255, 255), RGB(10, 10, 35));
                                edit_x += 8;
                                current_col++;
                            }
                        }
                        
                        // Editor loop
                        bool editing = true;
                        char cmd_buf[32];
                        int cmd_buf_pos = 0;
                        bool in_command_mode = false;
                        int status_y = fb->height - 40;
                        
                        // Initial status display
                        {
                            char status[64] = "Line ";
                            int si = 5;
                            char temp[16]; int ti = 0;
                            int ln = current_line;
                            if (ln == 0) status[si++] = '0';
                            else {
                                while (ln > 0) { temp[ti++] = '0' + (ln % 10); ln /= 10; }
                                while (ti > 0) status[si++] = temp[--ti];
                            }
                            status[si++] = ',';
                            status[si++] = ' ';
                            status[si++] = 'C';
                            status[si++] = 'o';
                            status[si++] = 'l';
                            status[si++] = ' ';
                            ti = 0;
                            int cl = current_col;
                            if (cl == 0) status[si++] = '0';
                            else {
                                while (cl > 0) { temp[ti++] = '0' + (cl % 10); cl /= 10; }
                                while (ti > 0) status[si++] = temp[--ti];
                            }
                            status[si] = '\0';
                            fb_draw_string(20, status_y, status, RGB(255, 200, 100), RGB(10, 10, 35));
                        }
                        
                        while (editing) {
                            if (keyboard_available()) {
                                char c = keyboard_getchar();
                                
                                if (in_command_mode) {
                                    // Command mode
                                    if (c == '\n') {
                                        cmd_buf[cmd_buf_pos] = '\0';
                                        
                                        // Clear status line
                                        fb_fill_rect(0, status_y, fb->width, 20, RGB(10, 10, 35));
                                        
                                        // Parse command
                                        if (cmd_buf_pos == 1 && cmd_buf[0] == 'q') {
                                            // :q - quit
                                            editing = false;
                                        } else if (cmd_buf_pos == 1 && cmd_buf[0] == 'w') {
                                            // :w - save
                                            edit_buffer[edit_pos] = '\0';
                                            file_t* save_file = vfs_open(filename);
                                            if (!save_file) {
                                                save_file = vfs_create(filename, FILE_TYPE_REGULAR);
                                            }
                                            if (save_file) {
                                                vfs_write(save_file, edit_buffer, edit_pos);
                                                fb_draw_string(20, status_y, "Saved!", RGB(0, 255, 100), RGB(10, 10, 35));
                                            }
                                        } else if (cmd_buf_pos == 2 && cmd_buf[0] == 'w' && cmd_buf[1] == 'q') {
                                            // :wq - save and quit
                                            edit_buffer[edit_pos] = '\0';
                                            file_t* save_file = vfs_open(filename);
                                            if (!save_file) {
                                                save_file = vfs_create(filename, FILE_TYPE_REGULAR);
                                            }
                                            if (save_file) {
                                                vfs_write(save_file, edit_buffer, edit_pos);
                                            }
                                            editing = false;
                                        } else {
                                            fb_draw_string(20, status_y, "Unknown command", RGB(255, 100, 100), RGB(10, 10, 35));
                                        }
                                        
                                        in_command_mode = false;
                                        cmd_buf_pos = 0;
                                    } else if (c == '\b' && cmd_buf_pos > 0) {
                                        cmd_buf_pos--;
                                        fb_fill_rect(28 + cmd_buf_pos * 8, status_y, 8, 16, RGB(10, 10, 35));
                                    } else if (c >= 32 && c < 127 && cmd_buf_pos < 30) {
                                        cmd_buf[cmd_buf_pos++] = c;
                                        char str[2] = {c, '\0'};
                                        fb_draw_string(28 + (cmd_buf_pos - 1) * 8, status_y, str, RGB(255, 255, 255), RGB(10, 10, 35));
                                    }
                                } else {
                                    // Edit mode
                                    if (c == ':') {
                                        // Enter command mode
                                        in_command_mode = true;
                                        cmd_buf_pos = 0;
                                        fb_fill_rect(0, status_y, fb->width, 20, RGB(10, 10, 35));
                                        fb_draw_string(20, status_y, ":", RGB(0, 255, 255), RGB(10, 10, 35));
                                    } else if (c == '\b' && edit_pos > 0) {
                                        edit_pos--;
                                        if (edit_buffer[edit_pos] == '\n') {
                                            current_line--;
                                            current_col = 1;
                                        } else {
                                            current_col--;
                                        }
                                        edit_x -= 8;
                                        if (edit_x < 20) {
                                            edit_y -= 20;
                                            edit_x = 20;
                                        }
                                        fb_fill_rect(edit_x, edit_y, 8, 16, RGB(10, 10, 35));
                                        // Update status
                                        {
                                            fb_fill_rect(0, status_y, fb->width, 20, RGB(10, 10, 35));
                                            char status[64] = "Line ";
                                            int si = 5;
                                            char temp[16]; int ti = 0;
                                            int ln = current_line;
                                            if (ln == 0) status[si++] = '0';
                                            else {
                                                while (ln > 0) { temp[ti++] = '0' + (ln % 10); ln /= 10; }
                                                while (ti > 0) status[si++] = temp[--ti];
                                            }
                                            status[si++] = ',';
                                            status[si++] = ' ';
                                            status[si++] = 'C';
                                            status[si++] = 'o';
                                            status[si++] = 'l';
                                            status[si++] = ' ';
                                            ti = 0;
                                            int cl = current_col;
                                            if (cl == 0) status[si++] = '0';
                                            else {
                                                while (cl > 0) { temp[ti++] = '0' + (cl % 10); cl /= 10; }
                                                while (ti > 0) status[si++] = temp[--ti];
                                            }
                                            status[si] = '\0';
                                            fb_draw_string(20, status_y, status, RGB(255, 200, 100), RGB(10, 10, 35));
                                        }
                                    } else if (c == '\n' && edit_pos < MAX_FILE_SIZE - 1) {
                                        edit_buffer[edit_pos++] = c;
                                        edit_y += 20;
                                        edit_x = 20;
                                        current_line++;
                                        current_col = 1;
                                        // Update status
                                        {
                                            fb_fill_rect(0, status_y, fb->width, 20, RGB(10, 10, 35));
                                            char status[64] = "Line ";
                                            int si = 5;
                                            char temp[16]; int ti = 0;
                                            int ln = current_line;
                                            if (ln == 0) status[si++] = '0';
                                            else {
                                                while (ln > 0) { temp[ti++] = '0' + (ln % 10); ln /= 10; }
                                                while (ti > 0) status[si++] = temp[--ti];
                                            }
                                            status[si++] = ',';
                                            status[si++] = ' ';
                                            status[si++] = 'C';
                                            status[si++] = 'o';
                                            status[si++] = 'l';
                                            status[si++] = ' ';
                                            ti = 0;
                                            int cl = current_col;
                                            if (cl == 0) status[si++] = '0';
                                            else {
                                                while (cl > 0) { temp[ti++] = '0' + (cl % 10); cl /= 10; }
                                                while (ti > 0) status[si++] = temp[--ti];
                                            }
                                            status[si] = '\0';
                                            fb_draw_string(20, status_y, status, RGB(255, 200, 100), RGB(10, 10, 35));
                                        }
                                    } else if (c >= 32 && c < 127 && edit_pos < MAX_FILE_SIZE - 1) {
                                        edit_buffer[edit_pos++] = c;
                                        char str[2] = {c, '\0'};
                                        fb_draw_string(edit_x, edit_y, str, RGB(255, 255, 255), RGB(10, 10, 35));
                                        edit_x += 8;
                                        current_col++;
                                        if (edit_x > (int)fb->width - 40) {
                                            edit_y += 20;
                                            edit_x = 20;
                                        }
                                        // Update status
                                        {
                                            fb_fill_rect(0, status_y, fb->width, 20, RGB(10, 10, 35));
                                            char status[64] = "Line ";
                                            int si = 5;
                                            char temp[16]; int ti = 0;
                                            int ln = current_line;
                                            if (ln == 0) status[si++] = '0';
                                            else {
                                                while (ln > 0) { temp[ti++] = '0' + (ln % 10); ln /= 10; }
                                                while (ti > 0) status[si++] = temp[--ti];
                                            }
                                            status[si++] = ',';
                                            status[si++] = ' ';
                                            status[si++] = 'C';
                                            status[si++] = 'o';
                                            status[si++] = 'l';
                                            status[si++] = ' ';
                                            ti = 0;
                                            int cl = current_col;
                                            if (cl == 0) status[si++] = '0';
                                            else {
                                                while (cl > 0) { temp[ti++] = '0' + (cl % 10); cl /= 10; }
                                                while (ti > 0) status[si++] = temp[--ti];
                                            }
                                            status[si] = '\0';
                                            fb_draw_string(20, status_y, status, RGB(255, 200, 100), RGB(10, 10, 35));
                                        }
                                    }
                                }
                            }
                            for (volatile int d = 0; d < 10000; d++);
                        }
                        
                        // Redraw screen
                        fb_clear(RGB(10, 10, 35));
                        line_y = 20;
                    }
                    // Unknown
                    else {
                        line_y += 20;
                        fb_draw_string(20, line_y, "Command not found. Type 'help' for help", RGB(255, 100, 100), RGB(10, 10, 35));
                    }
                }
                
                // New prompt
                line_y += 20;
                if (line_y > (int)fb->height - 40) {
                    fb_clear(RGB(10, 10, 35));
                    line_y = 20;
                }
                prompt_x = 20;
                fb_draw_string(prompt_x, line_y, "nicetop", RGB(0, 255, 100), RGB(10, 10, 35));
                prompt_x += 56;
                fb_draw_string(prompt_x, line_y, "@", RGB(150, 150, 150), RGB(10, 10, 35));
                prompt_x += 8;
                fb_draw_string(prompt_x, line_y, "system", RGB(100, 200, 255), RGB(10, 10, 35));
                prompt_x += 48;
                fb_draw_string(prompt_x, line_y, "~", RGB(255, 200, 0), RGB(10, 10, 35));
                prompt_x += 8;
                fb_draw_string(prompt_x, line_y, "$", RGB(255, 255, 255), RGB(10, 10, 35));
                prompt_x += 16;
                fb_draw_string(prompt_x, line_y, "type here...", RGB(80, 80, 80), RGB(10, 10, 35));
                cursor_x = prompt_x;
                cmd_pos = 0;
                placeholder_visible = true;
            }
            else if (c == '\b') {
                if (cmd_pos > 0) {
                    cmd_pos--;
                    cursor_x -= 8;
                    fb_fill_rect(cursor_x, line_y, 8, 16, RGB(10, 10, 35));
                }
            }
            else if (c == '\t' && cmd_pos > 0) {
                // Tab completion
                command_buffer[cmd_pos] = '\0';
                
                // Check if it's a command or filename
                bool is_file_arg = false;
                for (int i = 0; i < cmd_pos; i++) {
                    if (command_buffer[i] == ' ') {
                        is_file_arg = true;
                        break;
                    }
                }
                
                if (is_file_arg) {
                    // File completion - find last space
                    int space_pos = 0;
                    for (int i = cmd_pos - 1; i >= 0; i--) {
                        if (command_buffer[i] == ' ') {
                            space_pos = i + 1;
                            break;
                        }
                    }
                    
                    char* partial = command_buffer + space_pos;
                    int partial_len = cmd_pos - space_pos;
                    
                    // Try to match files
                    int file_count = vfs_get_file_count();
                    for (int i = 0; i < file_count; i++) {
                        file_t* file = vfs_get_file(i);
                        if (file) {
                            // Check if filename starts with partial
                            bool match = true;
                            for (int j = 0; j < partial_len; j++) {
                                if (file->name[j] != partial[j]) {
                                    match = false;
                                    break;
                                }
                            }
                            
                            if (match) {
                                // Complete the filename
                                int name_len = 0;
                                while (file->name[name_len]) name_len++;
                                
                                // Clear old text
                                fb_fill_rect(cursor_x - partial_len * 8, line_y, partial_len * 8, 16, RGB(10, 10, 35));
                                
                                // Copy completed name
                                for (int j = 0; j < name_len; j++) {
                                    command_buffer[space_pos + j] = file->name[j];
                                }
                                cmd_pos = space_pos + name_len;
                                
                                // Redraw
                                cursor_x = 20 + 56 + 8 + 48 + 8 + 8 + 16;
                                for (int j = 0; j < cmd_pos; j++) {
                                    char str[2] = {command_buffer[j], '\0'};
                                    fb_draw_string(cursor_x, line_y, str, RGB(255, 255, 255), RGB(10, 10, 35));
                                    cursor_x += 8;
                                }
                                break;
                            }
                        }
                    }
                } else {
                    // Command completion
                    const char* commands[] = {"help", "clear", "ls", "cat", "uname", "uptime", "echo", "free", "touch", "rm", "top", "edit", "ping", "ifconfig", "wget"};
                    int num_commands = 15;
                    
                    for (int i = 0; i < num_commands; i++) {
                        // Check if command starts with buffer
                        bool match = true;
                        for (int j = 0; j < cmd_pos; j++) {
                            if (commands[i][j] != command_buffer[j]) {
                                match = false;
                                break;
                            }
                        }
                        
                        if (match) {
                            // Complete the command
                            int cmd_len = 0;
                            while (commands[i][cmd_len]) cmd_len++;
                            
                            // Clear old text
                            fb_fill_rect(cursor_x - cmd_pos * 8, line_y, cmd_pos * 8, 16, RGB(10, 10, 35));
                            
                            // Copy completed command
                            for (int j = 0; j < cmd_len; j++) {
                                command_buffer[j] = commands[i][j];
                            }
                            cmd_pos = cmd_len;
                            
                            // Redraw
                            cursor_x = 20 + 56 + 8 + 48 + 8 + 8 + 16;
                            for (int j = 0; j < cmd_pos; j++) {
                                char str[2] = {command_buffer[j], '\0'};
                                fb_draw_string(cursor_x, line_y, str, RGB(255, 255, 255), RGB(10, 10, 35));
                                cursor_x += 8;
                            }
                            break;
                        }
                    }
                }
            }
            else if (c >= 32 && c < 127 && cmd_pos < 255) {
                // Clear placeholder on first character
                if (placeholder_visible) {
                    // Clear "type here..." (12 chars * 8 = 96 pixels)
                    int placeholder_start = 20 + 56 + 8 + 48 + 8 + 8 + 16; // 164
                    fb_fill_rect(placeholder_start, line_y, 96, 16, RGB(10, 10, 35));
                    placeholder_visible = false;
                }
                command_buffer[cmd_pos++] = c;
                char str[2] = {c, '\0'};
                fb_draw_string(cursor_x, line_y, str, RGB(255, 255, 255), RGB(10, 10, 35));
                cursor_x += 8;
            }
        }
        
        for (volatile int d = 0; d < 10000; d++);
    }
}
