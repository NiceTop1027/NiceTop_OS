#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// IDT entry structure
struct idt_entry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t always0;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed));

// IDT pointer structure
struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// Registers pushed by our ISR stub
struct registers {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
};

// Initialize IDT
void idt_init(void);

// Set IDT gate (for IRQ setup)
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

// ISR handler type
typedef void (*isr_t)(struct registers*);

// Register interrupt handler
void register_interrupt_handler(uint8_t n, isr_t handler);

// Interrupt handlers array (for IRQ)
extern isr_t interrupt_handlers[256];

#endif // IDT_H
