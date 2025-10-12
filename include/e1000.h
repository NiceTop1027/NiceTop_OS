#ifndef E1000_H
#define E1000_H

#include <stdint.h>

// E1000 Vendor and Device IDs
#define E1000_VENDOR_ID 0x8086
#define E1000_DEVICE_ID 0x100E

// E1000 Register offsets
#define E1000_REG_CTRL     0x0000
#define E1000_REG_STATUS   0x0008
#define E1000_REG_EECD     0x0010
#define E1000_REG_EERD     0x0014
#define E1000_REG_CTRL_EXT 0x0018
#define E1000_REG_MDIC     0x0020
#define E1000_REG_ICR      0x00C0
#define E1000_REG_IMS      0x00D0
#define E1000_REG_RCTL     0x0100
#define E1000_REG_TCTL     0x0400
#define E1000_REG_RDBAL    0x2800
#define E1000_REG_RDBAH    0x2804
#define E1000_REG_RDLEN    0x2808
#define E1000_REG_RDH      0x2810
#define E1000_REG_RDT      0x2818
#define E1000_REG_TDBAL    0x3800
#define E1000_REG_TDBAH    0x3804
#define E1000_REG_TDLEN    0x3808
#define E1000_REG_TDH      0x3810
#define E1000_REG_TDT      0x3818
#define E1000_REG_RAL      0x5400
#define E1000_REG_RAH      0x5404

// Control Register bits
#define E1000_CTRL_RST     0x04000000
#define E1000_CTRL_ASDE    0x00000020
#define E1000_CTRL_SLU     0x00000040

// Receive Control bits
#define E1000_RCTL_EN      0x00000002
#define E1000_RCTL_BAM     0x00008000
#define E1000_RCTL_BSIZE_2048 0x00000000

// Transmit Control bits
#define E1000_TCTL_EN      0x00000002
#define E1000_TCTL_PSP     0x00000008

// Descriptor counts
#define E1000_NUM_RX_DESC 32
#define E1000_NUM_TX_DESC 32

// Receive Descriptor
typedef struct {
    uint64_t addr;
    uint16_t length;
    uint16_t checksum;
    uint8_t status;
    uint8_t errors;
    uint16_t special;
} __attribute__((packed)) e1000_rx_desc_t;

// Transmit Descriptor
typedef struct {
    uint64_t addr;
    uint16_t length;
    uint8_t cso;
    uint8_t cmd;
    uint8_t status;
    uint8_t css;
    uint16_t special;
} __attribute__((packed)) e1000_tx_desc_t;

// E1000 functions
int e1000_driver_init(void);
void e1000_driver_send(uint8_t* data, uint32_t length);
int e1000_driver_receive(uint8_t* buffer, uint32_t max_length);
void e1000_get_mac(uint8_t* mac);

#endif // E1000_H
