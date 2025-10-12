#ifndef IRQ_H
#define IRQ_H

#include <stdint.h>

// Forward declaration
struct registers;
typedef void (*isr_t)(struct registers*);

// IRQ numbers
#define IRQ0  32    // Timer
#define IRQ1  33    // Keyboard
#define IRQ2  34
#define IRQ3  35
#define IRQ4  36
#define IRQ5  37
#define IRQ6  38
#define IRQ7  39
#define IRQ8  40
#define IRQ9  41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44    // Mouse
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47

// Initialize IRQs
void irq_init(void);

// Install IRQ handler
void irq_install_handler(int irq, isr_t handler);

// Uninstall IRQ handler
void irq_uninstall_handler(int irq);

#endif // IRQ_H
