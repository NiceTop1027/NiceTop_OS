#include "e1000.h"
#include "pci.h"
#include "serial.h"
#include "heap.h"
#include <stddef.h>

static uint8_t* mmio_addr = NULL;
static e1000_rx_desc_t* rx_descs = NULL;
static e1000_tx_desc_t* tx_descs = NULL;
static uint8_t** rx_buffers = NULL;
static uint8_t** tx_buffers = NULL;
static uint16_t rx_cur = 0;
static uint16_t tx_cur = 0;
static uint8_t mac_addr[6];

static void e1000_write_reg(uint16_t reg, uint32_t value) {
    if (!mmio_addr) return;
    *((volatile uint32_t*)(mmio_addr + reg)) = value;
}

static uint32_t e1000_read_reg(uint16_t reg) {
    if (!mmio_addr) return 0;
    return *((volatile uint32_t*)(mmio_addr + reg));
}

static void e1000_read_mac(void) {
    uint32_t low = e1000_read_reg(E1000_REG_RAL);
    uint32_t high = e1000_read_reg(E1000_REG_RAH);
    
    mac_addr[0] = low & 0xFF;
    mac_addr[1] = (low >> 8) & 0xFF;
    mac_addr[2] = (low >> 16) & 0xFF;
    mac_addr[3] = (low >> 24) & 0xFF;
    mac_addr[4] = high & 0xFF;
    mac_addr[5] = (high >> 8) & 0xFF;
}

int e1000_driver_init(void) {
    serial_write("E1000: Searching for device...\n");
    
    pci_init();
    pci_device_t* dev = pci_find_device(E1000_VENDOR_ID, E1000_DEVICE_ID);
    
    if (!dev) {
        serial_write("E1000: Device not found\n");
        return -1;
    }
    
    serial_write("E1000: Device found at bus 0\n");
    
    // Get MMIO address (must be non-zero)
    if (dev->bar0 == 0) {
        serial_write("E1000: Invalid BAR0\n");
        return -1;
    }
    
    mmio_addr = (uint8_t*)(uint32_t)(dev->bar0 & 0xFFFFFFF0);
    serial_write("E1000: MMIO configured\n");
    
    // Enable bus mastering and memory access
    uint32_t cmd = pci_read_config(dev->bus, dev->slot, dev->func, 0x04);
    cmd |= 0x07; // Bus master + Memory space + I/O space
    pci_write_config(dev->bus, dev->slot, dev->func, 0x04, cmd);
    serial_write("E1000: Bus mastering enabled\n");
    
    // Soft reset
    serial_write("E1000: Resetting device...\n");
    uint32_t ctrl = e1000_read_reg(E1000_REG_CTRL);
    e1000_write_reg(E1000_REG_CTRL, ctrl | E1000_CTRL_RST);
    
    // Wait for reset
    for (volatile int i = 0; i < 10000; i++);
    
    // Wait for device ready
    int timeout = 1000;
    while ((e1000_read_reg(E1000_REG_CTRL) & E1000_CTRL_RST) && timeout > 0) {
        for (volatile int i = 0; i < 100; i++);
        timeout--;
    }
    
    if (timeout == 0) {
        serial_write("E1000: Reset timeout\n");
        return -1;
    }
    
    serial_write("E1000: Reset complete\n");
    
    // Read MAC address
    e1000_read_mac();
    serial_write("E1000: MAC: ");
    for (int i = 0; i < 6; i++) {
        char hex[3];
        hex[0] = "0123456789ABCDEF"[mac_addr[i] >> 4];
        hex[1] = "0123456789ABCDEF"[mac_addr[i] & 0xF];
        hex[2] = '\0';
        serial_write(hex);
        if (i < 5) serial_write(":");
    }
    serial_write("\n");
    
    // Allocate descriptor rings
    serial_write("E1000: Allocating descriptors...\n");
    rx_descs = (e1000_rx_desc_t*)kmalloc(sizeof(e1000_rx_desc_t) * E1000_NUM_RX_DESC);
    tx_descs = (e1000_tx_desc_t*)kmalloc(sizeof(e1000_tx_desc_t) * E1000_NUM_TX_DESC);
    rx_buffers = (uint8_t**)kmalloc(sizeof(uint8_t*) * E1000_NUM_RX_DESC);
    tx_buffers = (uint8_t**)kmalloc(sizeof(uint8_t*) * E1000_NUM_TX_DESC);
    
    if (!rx_descs || !tx_descs || !rx_buffers || !tx_buffers) {
        serial_write("E1000: Failed to allocate descriptors\n");
        return -1;
    }
    
    // Allocate buffers
    for (int i = 0; i < E1000_NUM_RX_DESC; i++) {
        rx_buffers[i] = (uint8_t*)kmalloc(2048);
        rx_descs[i].addr = (uint64_t)(uint32_t)rx_buffers[i];
        rx_descs[i].status = 0;
    }
    
    for (int i = 0; i < E1000_NUM_TX_DESC; i++) {
        tx_buffers[i] = (uint8_t*)kmalloc(2048);
        tx_descs[i].addr = (uint64_t)(uint32_t)tx_buffers[i];
        tx_descs[i].status = 0;
        tx_descs[i].cmd = 0;
    }
    
    // Setup RX
    e1000_write_reg(E1000_REG_RDBAL, (uint32_t)rx_descs);
    e1000_write_reg(E1000_REG_RDBAH, 0);
    e1000_write_reg(E1000_REG_RDLEN, E1000_NUM_RX_DESC * sizeof(e1000_rx_desc_t));
    e1000_write_reg(E1000_REG_RDH, 0);
    e1000_write_reg(E1000_REG_RDT, E1000_NUM_RX_DESC - 1);
    e1000_write_reg(E1000_REG_RCTL, E1000_RCTL_EN | E1000_RCTL_BAM | E1000_RCTL_BSIZE_2048);
    
    // Setup TX
    e1000_write_reg(E1000_REG_TDBAL, (uint32_t)tx_descs);
    e1000_write_reg(E1000_REG_TDBAH, 0);
    e1000_write_reg(E1000_REG_TDLEN, E1000_NUM_TX_DESC * sizeof(e1000_tx_desc_t));
    e1000_write_reg(E1000_REG_TDH, 0);
    e1000_write_reg(E1000_REG_TDT, 0);
    e1000_write_reg(E1000_REG_TCTL, E1000_TCTL_EN | E1000_TCTL_PSP);
    
    // Link up
    e1000_write_reg(E1000_REG_CTRL, e1000_read_reg(E1000_REG_CTRL) | E1000_CTRL_SLU);
    
    serial_write("E1000: Initialized successfully\n");
    return 0;
}

void e1000_driver_send(uint8_t* data, uint32_t length) {
    if (!mmio_addr || !tx_descs) {
        serial_write("E1000: Not initialized\n");
        return;
    }
    
    // Copy data to buffer
    for (uint32_t i = 0; i < length && i < 2048; i++) {
        tx_buffers[tx_cur][i] = data[i];
    }
    
    // Setup descriptor
    tx_descs[tx_cur].length = length;
    tx_descs[tx_cur].cmd = 0x0B; // EOP | IFCS | RS
    tx_descs[tx_cur].status = 0;
    
    // Update tail
    uint16_t old_cur = tx_cur;
    tx_cur = (tx_cur + 1) % E1000_NUM_TX_DESC;
    e1000_write_reg(E1000_REG_TDT, tx_cur);
    
    // Wait for transmission with timeout
    int timeout = 10000;
    while (!(tx_descs[old_cur].status & 0x01) && timeout > 0) {
        for (volatile int i = 0; i < 100; i++);
        timeout--;
    }
    
    if (timeout > 0) {
        serial_write("E1000: Packet sent\n");
    } else {
        serial_write("E1000: Send timeout\n");
    }
}

int e1000_driver_receive(uint8_t* buffer, uint32_t max_length) {
    if (!mmio_addr || !rx_descs) return 0;
    
    if (!(rx_descs[rx_cur].status & 0x01)) {
        return 0; // No packet
    }
    
    uint16_t length = rx_descs[rx_cur].length;
    if (length > max_length) length = max_length;
    
    // Copy data
    for (uint16_t i = 0; i < length; i++) {
        buffer[i] = rx_buffers[rx_cur][i];
    }
    
    // Reset descriptor
    rx_descs[rx_cur].status = 0;
    
    // Update tail
    uint16_t old_cur = rx_cur;
    rx_cur = (rx_cur + 1) % E1000_NUM_RX_DESC;
    e1000_write_reg(E1000_REG_RDT, old_cur);
    
    return length;
}

void e1000_get_mac(uint8_t* mac) {
    for (int i = 0; i < 6; i++) {
        mac[i] = mac_addr[i];
    }
}
