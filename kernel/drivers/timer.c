#include "timer.h"
#include "irq.h"
#include "serial.h"

static uint32_t tick = 0;

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Timer interrupt handler
static void timer_handler(struct registers* regs) {
    (void)regs;
    tick++;
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void timer_init(uint32_t frequency) {
    serial_write("Timer: Initializing...\n");

    // Register timer handler
    irq_install_handler(0, timer_handler);

    // Calculate divisor
    uint32_t divisor = 1193180 / frequency;

    // Send command byte
    outb(0x43, 0x36);

    // Send frequency divisor
    uint8_t low = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);

    outb(0x40, low);
    outb(0x40, high);

    // Make sure timer IRQ is enabled in PIC
    uint8_t mask = inb(0x21);
    mask &= ~(1 << 0);  // Enable IRQ0 (timer)
    outb(0x21, mask);

    serial_write("Timer: IRQ enabled\n");
    serial_write("Timer: Initialized successfully\n");
}

uint32_t timer_get_ticks(void) {
    return tick;
}
