#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>

// Page size (4KB)
#define PAGE_SIZE 4096
#define PMM_BLOCK_SIZE PAGE_SIZE

// Physical memory manager functions
void pmm_init(uint32_t mem_low, uint32_t mem_high);
void pmm_init_region(uint32_t addr, uint32_t size);
void pmm_deinit_region(uint32_t addr, uint32_t size);

// Allocate and free physical memory blocks (4KB pages)
void* pmm_alloc_block(void);
void pmm_free_block(void* addr);

// Get memory information
uint32_t pmm_get_total_memory(void);
uint32_t pmm_get_used_blocks(void);
uint32_t pmm_get_free_blocks(void);
uint32_t pmm_get_memory_size(void);

#endif // PMM_H
