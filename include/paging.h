#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

// Page table entry flags
#define PAGE_PRESENT    0x01
#define PAGE_WRITE      0x02
#define PAGE_USER       0x04
#define PAGE_ACCESSED   0x20
#define PAGE_DIRTY      0x40

// Page directory/table sizes
#define PAGE_DIRECTORY_SIZE 1024
#define PAGE_TABLE_SIZE 1024

// Page directory entry
typedef struct {
    uint32_t present    : 1;
    uint32_t rw         : 1;
    uint32_t user       : 1;
    uint32_t accessed   : 1;
    uint32_t dirty      : 1;
    uint32_t unused     : 7;
    uint32_t frame      : 20;
} __attribute__((packed)) page_t;

// Page table
typedef struct {
    page_t pages[PAGE_TABLE_SIZE];
} page_table_t;

// Page directory
typedef struct {
    page_table_t* tables[PAGE_DIRECTORY_SIZE];
    uint32_t tables_physical[PAGE_DIRECTORY_SIZE];
    uint32_t physical_addr;
} page_directory_t;

// Initialize paging
void paging_init(void);

// Get current page directory
page_directory_t* paging_get_directory(void);

// Map a page
void paging_map_page(page_directory_t* dir, uint32_t virt, uint32_t phys, uint32_t flags);

// Unmap a page
void paging_unmap_page(page_directory_t* dir, uint32_t virt);

// Switch page directory
void paging_switch_directory(page_directory_t* dir);

#endif // PAGING_H
