#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>
#include <stddef.h>

// Heap memory allocation functions
void heap_init(void);
void* kmalloc(size_t size);
void* kmalloc_a(size_t size); // Page-aligned
void* kmalloc_p(size_t size, uint32_t* phys); // Returns physical address
void* kmalloc_ap(size_t size, uint32_t* phys); // Page-aligned + physical
void kfree(void* ptr);
void heap_stats(uint32_t* total, uint32_t* used, uint32_t* free_blocks);

#endif // HEAP_H
