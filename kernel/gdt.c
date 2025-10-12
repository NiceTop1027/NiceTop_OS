#include "gdt.h"
#include "serial.h"

// GDT entries
gdt_entry_t gdt[3];
gdt_ptr_t gp;

// External assembly function to load GDT
extern void gdt_flush(uint32_t);

// Set a GDT entry
static void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);
    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access = access;
}

void gdt_init(void) {
    serial_write("GDT: Initializing...\n");

    // Setup GDT pointer
    gp.limit = (sizeof(gdt_entry_t) * 3) - 1;
    gp.base = (uint32_t)&gdt;

    // NULL descriptor
    gdt_set_gate(0, 0, 0, 0, 0);

    // Code segment: base=0, limit=4GB, access=0x9A, granularity=0xCF
    // Access: Present, Ring 0, Code segment, Executable, Readable
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    // Data segment: base=0, limit=4GB, access=0x92, granularity=0xCF
    // Access: Present, Ring 0, Data segment, Writable
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    // Load the GDT
    gdt_flush((uint32_t)&gp);

    serial_write("GDT: Initialized successfully\n");
}
