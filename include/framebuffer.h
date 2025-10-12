#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>

typedef struct {
    uint32_t* address;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint8_t bpp;
} framebuffer_info_t;

// Initialize framebuffer
void framebuffer_init(uint32_t* addr, uint32_t width, uint32_t height, uint32_t pitch, uint8_t bpp);

// Get framebuffer info
framebuffer_info_t* framebuffer_get_info(void);

// Draw pixel
void fb_putpixel(uint32_t x, uint32_t y, uint32_t color);

// Draw rectangle
void fb_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);

// Fill rectangle
void fb_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);

// Clear screen
void fb_clear(uint32_t color);

// Draw character (8x16 font)
void fb_draw_char(uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg);

// Draw string
void fb_draw_string(uint32_t x, uint32_t y, const char* str, uint32_t fg, uint32_t bg);

// RGB color helper
#define RGB(r, g, b) (((r) << 16) | ((g) << 8) | (b))

// Common colors
#define COLOR_BLACK   RGB(0, 0, 0)
#define COLOR_WHITE   RGB(255, 255, 255)
#define COLOR_RED     RGB(255, 0, 0)
#define COLOR_GREEN   RGB(0, 255, 0)
#define COLOR_BLUE    RGB(0, 0, 255)
#define COLOR_YELLOW  RGB(255, 255, 0)
#define COLOR_CYAN    RGB(0, 255, 255)
#define COLOR_MAGENTA RGB(255, 0, 255)
#define COLOR_GRAY    RGB(128, 128, 128)
#define COLOR_ORANGE  RGB(255, 165, 0)

#endif // FRAMEBUFFER_H
