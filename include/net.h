#ifndef NET_H
#define NET_H

#include <stdint.h>

// Ethernet frame
typedef struct {
    uint8_t dest_mac[6];
    uint8_t src_mac[6];
    uint16_t ethertype;
} __attribute__((packed)) eth_header_t;

// IP header
typedef struct {
    uint8_t version_ihl;
    uint8_t tos;
    uint16_t total_length;
    uint16_t id;
    uint16_t flags_fragment;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    uint32_t src_ip;
    uint32_t dest_ip;
} __attribute__((packed)) ip_header_t;

// ICMP header
typedef struct {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t id;
    uint16_t sequence;
} __attribute__((packed)) icmp_header_t;

// ARP packet
typedef struct {
    uint16_t hw_type;
    uint16_t proto_type;
    uint8_t hw_size;
    uint8_t proto_size;
    uint16_t opcode;
    uint8_t sender_mac[6];
    uint32_t sender_ip;
    uint8_t target_mac[6];
    uint32_t target_ip;
} __attribute__((packed)) arp_packet_t;

// Network interface
typedef struct {
    uint8_t mac[6];
    uint32_t ip;
    uint32_t netmask;
    uint32_t gateway;
} net_interface_t;

// Network functions
int net_init(void);
void net_set_ip(uint32_t ip, uint32_t netmask, uint32_t gateway);
void net_get_mac(uint8_t* mac);
void net_send_packet(uint8_t* data, uint32_t length);
void net_receive_packet(uint8_t* data, uint32_t length);
int net_ping(uint32_t dest_ip);
uint16_t net_checksum(void* data, int length);

// E1000 driver functions
void e1000_init(void);
void e1000_send(uint8_t* data, uint32_t length);
int e1000_receive(uint8_t* buffer, uint32_t max_length);

// Utility functions
uint32_t ip_from_string(const char* str);
void ip_to_string(uint32_t ip, char* str);

// HTTP functions
int net_http_get(const char* url, char* buffer, int max_size);

#endif // NET_H
