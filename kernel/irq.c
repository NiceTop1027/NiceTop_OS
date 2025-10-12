#include "irq.h"
#include "serial.h"
#include <stddef.h>

// Registers structure
struct registers {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
};

// External functions from idt.c
extern void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);
extern void register_interrupt_handler(uint8_t n, isr_t handler);
extern isr_t interrupt_handlers[256];

// PIC ports
#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

// PIC commands
#define PIC_EOI      0x20

// External IRQ handlers from assembly
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Remap PIC to avoid conflicts with CPU exceptions
static void pic_remap(void) {
    // Save masks
    uint8_t mask1 = 0;
    uint8_t mask2 = 0;

    // Start initialization
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);

    // Set vector offsets
    outb(PIC1_DATA, 0x20);  // IRQ 0-7 mapped to 32-39
    outb(PIC2_DATA, 0x28);  // IRQ 8-15 mapped to 40-47

    // Tell PICs about each other
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);

    // Set mode
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    // Restore masks (enable all IRQs)
    outb(PIC1_DATA, 0x00);
    outb(PIC2_DATA, 0x00);
}

void irq_init(void) {
    serial_write("IRQ: Initializing...\n");

    // Remap PIC
    pic_remap();

    // Install IRQ handlers
    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E);
    idt_set_gate(34, (uint32_t)irq2, 0x08, 0x8E);
    idt_set_gate(35, (uint32_t)irq3, 0x08, 0x8E);
    idt_set_gate(36, (uint32_t)irq4, 0x08, 0x8E);
    idt_set_gate(37, (uint32_t)irq5, 0x08, 0x8E);
    idt_set_gate(38, (uint32_t)irq6, 0x08, 0x8E);
    idt_set_gate(39, (uint32_t)irq7, 0x08, 0x8E);
    idt_set_gate(40, (uint32_t)irq8, 0x08, 0x8E);
    idt_set_gate(41, (uint32_t)irq9, 0x08, 0x8E);
    idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E);

    // Enable interrupts
    __asm__ volatile("sti");

    serial_write("IRQ: Initialized successfully\n");
}

void irq_install_handler(int irq, isr_t handler) {
    register_interrupt_handler(IRQ0 + irq, handler);
}

void irq_uninstall_handler(int irq) {
    register_interrupt_handler(IRQ0 + irq, NULL);
}

// IRQ handler called from assembly
void irq_handler(struct registers* regs) {
    // Send EOI to PICs
    if (regs->int_no >= 40) {
        outb(PIC2_COMMAND, PIC_EOI);  // Send to slave
    }
    outb(PIC1_COMMAND, PIC_EOI);      // Send to master

    // Call registered handler
    if (interrupt_handlers[regs->int_no] != NULL) {
        isr_t handler = interrupt_handlers[regs->int_no];
        handler(regs);
    }
}
