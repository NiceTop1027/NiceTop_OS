#include "net.h"
#include "e1000.h"
#include "serial.h"
#include <stddef.h>

static net_interface_t net_if;

int net_init(void) {
    serial_write("Network: Initializing...\n");
    
    // Initialize E1000 driver
    int result = e1000_driver_init();
    if (result == 0) {
        // Get MAC from hardware
        e1000_get_mac(net_if.mac);
        serial_write("Network: Using hardware MAC\n");
    } else {
        serial_write("Network: E1000 init failed, using defaults\n");
        // Fallback MAC address
        net_if.mac[0] = 0x52;
        net_if.mac[1] = 0x54;
        net_if.mac[2] = 0x00;
        net_if.mac[3] = 0x12;
        net_if.mac[4] = 0x34;
        net_if.mac[5] = 0x56;
    }
    
    // Default IP: 10.0.2.15 (QEMU default)
    net_if.ip = (10 << 24) | (0 << 16) | (2 << 8) | 15;
    net_if.netmask = (255 << 24) | (255 << 16) | (255 << 8) | 0;
    net_if.gateway = (10 << 24) | (0 << 16) | (2 << 8) | 2;
    
    serial_write("Network: Initialized\n");
    return result;
}

void net_set_ip(uint32_t ip, uint32_t netmask, uint32_t gateway) {
    net_if.ip = ip;
    net_if.netmask = netmask;
    net_if.gateway = gateway;
}

void net_get_mac(uint8_t* mac) {
    for (int i = 0; i < 6; i++) {
        mac[i] = net_if.mac[i];
    }
}

uint16_t net_checksum(void* data, int length) {
    uint32_t sum = 0;
    uint16_t* ptr = (uint16_t*)data;
    
    while (length > 1) {
        sum += *ptr++;
        length -= 2;
    }
    
    if (length > 0) {
        sum += *(uint8_t*)ptr;
    }
    
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return ~sum;
}

void net_send_packet(uint8_t* data, uint32_t length) {
    // Send via E1000 hardware
    e1000_driver_send(data, length);
    
    serial_write("Network: Sent ");
    char buf[16];
    int i = 0;
    uint32_t len = length;
    if (len == 0) {
        serial_write("0");
    } else {
        while (len > 0) {
            buf[i++] = '0' + (len % 10);
            len /= 10;
        }
        buf[i] = '\0';
        // Reverse the string
        for (int j = 0; j < i / 2; j++) {
            char tmp = buf[j];
            buf[j] = buf[i - j - 1];
            buf[i - j - 1] = tmp;
        }
        serial_write(buf);
    }
    serial_write(" bytes\n");
}

void net_receive_packet(uint8_t* data, uint32_t length) {
    (void)data;
    (void)length;
    // Process received packet
}

int net_ping(uint32_t dest_ip) {
    serial_write("Network: Sending ICMP echo request to ");
    
    char ip_str[16];
    ip_to_string(dest_ip, ip_str);
    serial_write(ip_str);
    serial_write("\n");
    
    // Build ICMP packet
    uint8_t packet[128];
    eth_header_t* eth = (eth_header_t*)packet;
    ip_header_t* ip = (ip_header_t*)(packet + sizeof(eth_header_t));
    icmp_header_t* icmp = (icmp_header_t*)(packet + sizeof(eth_header_t) + sizeof(ip_header_t));
    
    // Ethernet header (broadcast for simplicity)
    for (int i = 0; i < 6; i++) {
        eth->dest_mac[i] = 0xFF;
        eth->src_mac[i] = net_if.mac[i];
    }
    eth->ethertype = 0x0008; // IP (big endian)
    
    // IP header
    ip->version_ihl = 0x45; // IPv4, 20 bytes
    ip->tos = 0;
    ip->total_length = sizeof(ip_header_t) + sizeof(icmp_header_t);
    ip->id = 0x1234;
    ip->flags_fragment = 0;
    ip->ttl = 64;
    ip->protocol = 1; // ICMP
    ip->src_ip = net_if.ip;
    ip->dest_ip = dest_ip;
    ip->checksum = 0;
    ip->checksum = net_checksum(ip, sizeof(ip_header_t));
    
    // ICMP header
    icmp->type = 8; // Echo request
    icmp->code = 0;
    icmp->id = 0x1234;
    icmp->sequence = 1;
    icmp->checksum = 0;
    icmp->checksum = net_checksum(icmp, sizeof(icmp_header_t));
    
    uint32_t total_length = sizeof(eth_header_t) + sizeof(ip_header_t) + sizeof(icmp_header_t);
    net_send_packet(packet, total_length);
    
    return 0;
}

uint32_t ip_from_string(const char* str) {
    uint32_t ip = 0;
    uint32_t octet = 0;
    int shift = 24;
    
    for (int i = 0; str[i]; i++) {
        if (str[i] >= '0' && str[i] <= '9') {
            octet = octet * 10 + (str[i] - '0');
        } else if (str[i] == '.') {
            ip |= (octet << shift);
            shift -= 8;
            octet = 0;
        }
    }
    ip |= (octet << shift);
    
    return ip;
}

void ip_to_string(uint32_t ip, char* str) {
    int pos = 0;
    
    for (int i = 3; i >= 0; i--) {
        uint8_t octet = (ip >> (i * 8)) & 0xFF;
        
        if (octet >= 100) {
            str[pos++] = '0' + (octet / 100);
            octet %= 100;
            str[pos++] = '0' + (octet / 10);
            str[pos++] = '0' + (octet % 10);
        } else if (octet >= 10) {
            str[pos++] = '0' + (octet / 10);
            str[pos++] = '0' + (octet % 10);
        } else {
            str[pos++] = '0' + octet;
        }
        
        if (i > 0) str[pos++] = '.';
    }
    
    str[pos] = '\0';
}

// Legacy wrappers for compatibility
void e1000_init(void) {
    // Now handled by e1000_driver_init() in net_init()
}

void e1000_send(uint8_t* data, uint32_t length) {
    e1000_driver_send(data, length);
}

int e1000_receive(uint8_t* buffer, uint32_t max_length) {
    return e1000_driver_receive(buffer, max_length);
}

// Simple HTTP GET implementation (simulated)
int net_http_get(const char* url, char* buffer, int max_size) {
    serial_write("HTTP: GET ");
    serial_write(url);
    serial_write("\n");
    
    // Simulated response
    const char* response = 
        "HTTP/1.1 200 OK\n"
        "Content-Type: text/html\n"
        "\n"
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head><title>NiceTop OS</title></head>\n"
        "<body>\n"
        "<h1>Welcome to NiceTop OS!</h1>\n"
        "<p>This is a simulated HTTP response.</p>\n"
        "<p>Requested URL: ";
    
    int i = 0;
    while (response[i] && i < max_size - 1) {
        buffer[i] = response[i];
        i++;
    }
    
    // Add URL to response
    int j = 0;
    while (url[j] && i < max_size - 20) {
        buffer[i++] = url[j++];
    }
    
    const char* end = "</p>\n</body>\n</html>\n";
    j = 0;
    while (end[j] && i < max_size - 1) {
        buffer[i++] = end[j++];
    }
    
    buffer[i] = '\0';
    return i;
}
