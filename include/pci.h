#ifndef PCI_H
#define PCI_H

#include <stdint.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

// PCI device structure
typedef struct {
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t bus;
    uint8_t slot;
    uint8_t func;
    uint32_t bar0;
    uint32_t bar1;
    uint8_t irq;
} pci_device_t;

// PCI functions
void pci_init(void);
uint32_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void pci_write_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value);
pci_device_t* pci_find_device(uint16_t vendor_id, uint16_t device_id);
uint32_t pci_get_bar(uint8_t bus, uint8_t slot, uint8_t func, uint8_t bar_num);

#endif // PCI_H
