#include "keyboard.h"
#include "irq.h"
#include "serial.h"
#include "vga.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

// Keyboard buffer
#define BUFFER_SIZE 256
static char key_buffer[BUFFER_SIZE];
static int buffer_start = 0;
static int buffer_end = 0;

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// US QWERTY keyboard layout - Extended
static const char scancode_to_ascii[128] = {
    0,    0,    '1',  '2',  '3',  '4',  '5',  '6',   // 0-7
    '7',  '8',  '9',  '0',  '-',  '=',  '\b', '\t',  // 8-15
    'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',   // 16-23
    'o',  'p',  '[',  ']',  '\n', 0,    'a',  's',   // 24-31
    'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',   // 32-39
    '\'', '`',  0,    '\\', 'z',  'x',  'c',  'v',   // 40-47
    'b',  'n',  'm',  ',',  '.',  '/',  0,    '*',   // 48-55
    0,    ' ',  0,    0,    0,    0,    0,    0,     // 56-63
    0,    0,    0,    0,    0,    0,    0,    '7',   // 64-71 (F-keys, numpad)
    '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',   // 72-79
    '2',  '3',  '0',  '.',  0,    0,    0,    0,     // 80-87
    0,    0,    0,    0,    0,    0,    0,    0,     // 88-95
    0,    0,    0,    0,    0,    0,    0,    0,     // 96-103
    0,    0,    0,    0,    0,    0,    0,    0,     // 104-111
    0,    0,    0,    0,    0,    0,    0,    0,     // 112-119
    0,    0,    0,    0,    0,    0,    0,    0      // 120-127
};

static const char scancode_to_ascii_shift[128] = {
    0,    0,    '!',  '@',  '#',  '$',  '%',  '^',   // 0-7
    '&',  '*',  '(',  ')',  '_',  '+',  '\b', '\t',  // 8-15
    'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',   // 16-23
    'O',  'P',  '{',  '}',  '\n', 0,    'A',  'S',   // 24-31
    'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',   // 32-39
    '"',  '~',  0,    '|',  'Z',  'X',  'C',  'V',   // 40-47
    'B',  'N',  'M',  '<',  '>',  '?',  0,    '*',   // 48-55
    0,    ' ',  0,    0,    0,    0,    0,    0,     // 56-63
    0,    0,    0,    0,    0,    0,    0,    '7',   // 64-71
    '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',   // 72-79
    '2',  '3',  '0',  '.',  0,    0,    0,    0,     // 80-87
    0,    0,    0,    0,    0,    0,    0,    0,     // 88-95
    0,    0,    0,    0,    0,    0,    0,    0,     // 96-103
    0,    0,    0,    0,    0,    0,    0,    0,     // 104-111
    0,    0,    0,    0,    0,    0,    0,    0,     // 112-119
    0,    0,    0,    0,    0,    0,    0,    0      // 120-127
};

static int shift_pressed = 0;

// Add key to buffer
static void buffer_add(char c) {
    int next = (buffer_end + 1) % BUFFER_SIZE;
    if (next != buffer_start) {
        key_buffer[buffer_end] = c;
        buffer_end = next;
    }
}

// Get key from buffer
static char buffer_get(void) {
    if (buffer_start == buffer_end) {
        return 0;
    }
    char c = key_buffer[buffer_start];
    buffer_start = (buffer_start + 1) % BUFFER_SIZE;
    return c;
}

// Keyboard interrupt handler
static void keyboard_handler(struct registers* regs) {
    (void)regs;
    
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    
    // Check for shift press/release
    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = 1;
        return;
    }
    if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = 0;
        return;
    }
    
    // Ignore key releases (high bit set)
    if (scancode & 0x80) {
        return;
    }
    
    // Handle special keys
    char c = 0;
    
    if (scancode == 0x0E) {
        // Backspace
        c = '\b';
    } else if (scancode == 0x1C) {
        // Enter
        c = '\n';
    } else if (scancode == 0x48) {
        // Up arrow
        c = 0x10; // Special code for up
    } else if (scancode == 0x50) {
        // Down arrow
        c = 0x11; // Special code for down
    } else if (scancode == 0x4B) {
        // Left arrow
        c = 0x12; // Special code for left
    } else if (scancode == 0x4D) {
        // Right arrow
        c = 0x13; // Special code for right
    } else if (scancode < sizeof(scancode_to_ascii)) {
        // Regular key
        if (shift_pressed) {
            c = scancode_to_ascii_shift[scancode];
        } else {
            c = scancode_to_ascii[scancode];
        }
    }
    
    if (c != 0) {
        buffer_add(c);
        
        // Echo to screen
        if (c == '\b') {
            vga_putchar('\b');
        } else if (c == '\n') {
            vga_putchar('\n');
        } else {
            vga_putchar(c);
        }
    }
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static void keyboard_wait_input(void) {
    // Wait for input buffer to be clear
    int timeout = 100000;
    while (timeout-- > 0) {
        if (!(inb(KEYBOARD_STATUS_PORT) & 0x02)) {
            return;
        }
    }
}

static void keyboard_wait_output(void) {
    // Wait for output buffer to have data
    int timeout = 100000;
    while (timeout-- > 0) {
        if (inb(KEYBOARD_STATUS_PORT) & 0x01) {
            return;
        }
    }
}

void keyboard_init(void) {
    serial_write("Keyboard: Initializing...\n");

    // Disable first PS/2 port
    keyboard_wait_input();
    outb(KEYBOARD_STATUS_PORT, 0xAD);

    // Flush output buffer
    inb(KEYBOARD_DATA_PORT);

    // Set controller configuration
    keyboard_wait_input();
    outb(KEYBOARD_STATUS_PORT, 0x20);  // Read configuration
    keyboard_wait_output();
    uint8_t config = inb(KEYBOARD_DATA_PORT);
    config |= 0x01;  // Enable keyboard interrupt
    config &= ~0x10; // Enable keyboard
    keyboard_wait_input();
    outb(KEYBOARD_STATUS_PORT, 0x60);  // Write configuration
    keyboard_wait_input();
    outb(KEYBOARD_DATA_PORT, config);

    // Enable first PS/2 port
    keyboard_wait_input();
    outb(KEYBOARD_STATUS_PORT, 0xAE);

    // Reset and enable keyboard
    keyboard_wait_input();
    outb(KEYBOARD_DATA_PORT, 0xFF);  // Reset command
    keyboard_wait_output();
    uint8_t response = inb(KEYBOARD_DATA_PORT);  // Should be 0xFA (ACK) or 0xAA (self-test passed)

    if (response == 0xFA) {
        keyboard_wait_output();
        response = inb(KEYBOARD_DATA_PORT);  // Get actual response
    }

    serial_write("Keyboard: Reset response = 0x");
    char hex[3];
    hex[0] = "0123456789ABCDEF"[response >> 4];
    hex[1] = "0123456789ABCDEF"[response & 0x0F];
    hex[2] = '\0';
    serial_write(hex);
    serial_write("\n");

    // Enable scanning
    keyboard_wait_input();
    outb(KEYBOARD_DATA_PORT, 0xF4);  // Enable scanning
    keyboard_wait_output();
    response = inb(KEYBOARD_DATA_PORT);  // Should be 0xFA (ACK)

    // Register keyboard interrupt handler (IRQ1)
    irq_install_handler(1, keyboard_handler);

    // Make sure keyboard IRQ is enabled in PIC
    uint8_t mask = inb(0x21);
    mask &= ~(1 << 1);  // Enable IRQ1 (keyboard)
    outb(0x21, mask);

    serial_write("Keyboard: IRQ enabled\n");
    serial_write("Keyboard: Initialized successfully\n");
}

// Poll keyboard (fallback if interrupts don't work)
static char keyboard_poll(void) {
    // Check if data is available
    uint8_t status = inb(KEYBOARD_STATUS_PORT);
    if (!(status & 0x01)) {
        return 0;  // No data
    }
    
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    
    // Check for shift press/release
    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = 1;
        return 0;
    }
    if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = 0;
        return 0;
    }
    
    // Ignore key releases
    if (scancode & 0x80) {
        return 0;
    }
    
    // Handle special keys
    char c = 0;
    if (scancode == 0x0E) {
        c = '\b';
    } else if (scancode == 0x1C) {
        c = '\n';
    } else if (scancode < sizeof(scancode_to_ascii)) {
        if (shift_pressed) {
            c = scancode_to_ascii_shift[scancode];
        } else {
            c = scancode_to_ascii[scancode];
        }
    }
    
    return c;
}

char keyboard_getchar(void) {
    // Try interrupt-based first
    if (buffer_start != buffer_end) {
        return buffer_get();
    }
    
    // Fall back to polling
    while (1) {
        char c = keyboard_poll();
        if (c != 0) {
            // Echo to screen
            if (c == '\b') {
                vga_putchar('\b');
            } else if (c == '\n') {
                vga_putchar('\n');
            } else {
                vga_putchar(c);
            }
            return c;
        }
        __asm__ volatile("hlt");
    }
}

int keyboard_available(void) {
    // Check buffer first
    if (buffer_start != buffer_end) {
        return 1;
    }
    
    // Poll for new data
    uint8_t status = inb(KEYBOARD_STATUS_PORT);
    if (status & 0x01) {
        // Data available, read it
        char c = keyboard_poll();
        if (c != 0) {
            buffer_add(c);
            return 1;
        }
    }
    
    return 0;
}
