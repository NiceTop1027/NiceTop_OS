#include "vga.h"

static uint16_t* vga_buffer = (uint16_t*)VGA_MEMORY;
static size_t vga_row = 0;
static size_t vga_column = 0;
static uint8_t vga_color = 0x0F; // White on black

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Update hardware cursor position
static void update_cursor(void) {
    uint16_t pos = vga_row * VGA_WIDTH + vga_column;
    
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

static inline uint8_t vga_entry_color(uint8_t fg, uint8_t bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char c, uint8_t color) {
    return (uint16_t)c | (uint16_t)color << 8;
}

void vga_init(void) {
    vga_row = 0;
    vga_column = 0;
    vga_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
}

void vga_clear(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            vga_buffer[index] = vga_entry(' ', vga_color);
        }
    }
    vga_row = 0;
    vga_column = 0;
    update_cursor();
}

void vga_set_color(uint8_t fg, uint8_t bg) {
    vga_color = vga_entry_color(fg, bg);
}

static void vga_scroll(void) {
    // Move all lines up
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    
    // Clear last line
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', vga_color);
    }
    
    vga_row = VGA_HEIGHT - 1;
}

void vga_putchar(char c) {
    if (c == '\n') {
        vga_column = 0;
        if (++vga_row == VGA_HEIGHT) {
            vga_scroll();
        }
        update_cursor();
        return;
    }
    
    if (c == '\r') {
        vga_column = 0;
        update_cursor();
        return;
    }
    
    if (c == '\b') {
        // Backspace
        if (vga_column > 0) {
            vga_column--;
            const size_t index = vga_row * VGA_WIDTH + vga_column;
            vga_buffer[index] = vga_entry(' ', vga_color);
        }
        update_cursor();
        return;
    }
    
    if (c == '\t') {
        vga_column = (vga_column + 4) & ~3;
        if (vga_column >= VGA_WIDTH) {
            vga_column = 0;
            if (++vga_row == VGA_HEIGHT) {
                vga_scroll();
            }
        }
        update_cursor();
        return;
    }
    
    const size_t index = vga_row * VGA_WIDTH + vga_column;
    vga_buffer[index] = vga_entry(c, vga_color);
    
    if (++vga_column == VGA_WIDTH) {
        vga_column = 0;
        if (++vga_row == VGA_HEIGHT) {
            vga_scroll();
        }
    }
    update_cursor();
}

void vga_write(const char* str) {
    while (*str) {
        vga_putchar(*str++);
    }
}

void vga_write_char(char c) {
    vga_putchar(c);
}

void vga_write_color(const char* str, uint8_t fg, uint8_t bg) {
    uint8_t old_color = vga_color;
    vga_set_color(fg, bg);
    vga_write(str);
    vga_color = old_color;
}

void vga_set_cursor(size_t row, size_t col) {
    if (row >= VGA_HEIGHT) row = VGA_HEIGHT - 1;
    if (col >= VGA_WIDTH) col = VGA_WIDTH - 1;
    vga_row = row;
    vga_column = col;
    update_cursor();
}
