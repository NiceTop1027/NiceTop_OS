#include "pci.h"
#include "serial.h"

static inline void outl(uint16_t port, uint32_t val) {
    __asm__ volatile ("outl %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    __asm__ volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void pci_init(void) {
    serial_write("PCI: Initializing...\n");
}

uint32_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC) | 0x80000000);
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

void pci_write_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value) {
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC) | 0x80000000);
    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, value);
}

pci_device_t* pci_find_device(uint16_t vendor_id, uint16_t device_id) {
    static pci_device_t device;
    
    // Only scan bus 0 for speed
    for (uint8_t slot = 0; slot < 32; slot++) {
        uint32_t config = pci_read_config(0, slot, 0, 0);
        uint16_t vid = config & 0xFFFF;
        uint16_t did = (config >> 16) & 0xFFFF;
        
        if (vid == 0xFFFF) continue;
        
        if (vid == vendor_id && did == device_id) {
            device.vendor_id = vid;
            device.device_id = did;
            device.bus = 0;
            device.slot = slot;
            device.func = 0;
            device.bar0 = pci_get_bar(0, slot, 0, 0);
            device.bar1 = pci_get_bar(0, slot, 0, 1);
            
            // Get IRQ
            uint32_t irq_config = pci_read_config(0, slot, 0, 0x3C);
            device.irq = irq_config & 0xFF;
            
            serial_write("PCI: Found device at slot ");
            return &device;
        }
    }
    
    return 0;
}

uint32_t pci_get_bar(uint8_t bus, uint8_t slot, uint8_t func, uint8_t bar_num) {
    uint32_t bar = pci_read_config(bus, slot, func, 0x10 + (bar_num * 4));
    return bar & 0xFFFFFFF0; // Mask off lower bits
}
