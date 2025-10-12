#include "heap.h"
#include "serial.h"
#include <stddef.h>

// Simple heap implementation
#define HEAP_START 0x00100000  // 1MB
#define HEAP_SIZE  0x00100000  // 1MB heap

typedef struct heap_block {
    size_t size;
    int is_free;
    struct heap_block* next;
} heap_block_t;

static uint32_t heap_current = HEAP_START;
static heap_block_t* heap_head = NULL;

void heap_init(void) {
    serial_write("Heap: Initializing at 0x00100000...\n");
    heap_head = (heap_block_t*)HEAP_START;
    heap_head->size = HEAP_SIZE - sizeof(heap_block_t);
    heap_head->is_free = 1;
    heap_head->next = NULL;
    heap_current = HEAP_START + sizeof(heap_block_t);
    serial_write("Heap: Initialized successfully\n");
}

void* kmalloc(size_t size) {
    if (size == 0) return NULL;
    
    // Align to 4 bytes
    size = (size + 3) & ~3;
    
    heap_block_t* current = heap_head;
    
    // First-fit algorithm
    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            // Found a suitable block
            current->is_free = 0;
            
            // Split block if there's enough space
            if (current->size > size + sizeof(heap_block_t) + 16) {
                heap_block_t* new_block = (heap_block_t*)((uint8_t*)current + sizeof(heap_block_t) + size);
                new_block->size = current->size - size - sizeof(heap_block_t);
                new_block->is_free = 1;
                new_block->next = current->next;
                
                current->size = size;
                current->next = new_block;
            }
            
            return (void*)((uint8_t*)current + sizeof(heap_block_t));
        }
        current = current->next;
    }
    
    // No suitable block found
    return NULL;
}

void* kmalloc_a(size_t size) {
    // Page-aligned allocation (4KB)
    uint32_t addr = heap_current;
    if (addr & 0xFFF) {
        addr = (addr & 0xFFFFF000) + 0x1000;
    }
    
    void* ptr = (void*)addr;
    heap_current = addr + size;
    
    return ptr;
}

void* kmalloc_p(size_t size, uint32_t* phys) {
    void* ptr = kmalloc(size);
    if (phys) {
        *phys = (uint32_t)ptr;
    }
    return ptr;
}

void* kmalloc_ap(size_t size, uint32_t* phys) {
    void* ptr = kmalloc_a(size);
    if (phys) {
        *phys = (uint32_t)ptr;
    }
    return ptr;
}

void kfree(void* ptr) {
    if (ptr == NULL) return;
    
    heap_block_t* block = (heap_block_t*)((uint8_t*)ptr - sizeof(heap_block_t));
    block->is_free = 1;
    
    // Coalesce with next block if it's free
    if (block->next && block->next->is_free) {
        block->size += sizeof(heap_block_t) + block->next->size;
        block->next = block->next->next;
    }
    
    // Coalesce with previous block if it's free
    heap_block_t* current = heap_head;
    while (current && current->next != block) {
        current = current->next;
    }
    
    if (current && current->is_free) {
        current->size += sizeof(heap_block_t) + block->size;
        current->next = block->next;
    }
}

// Get heap statistics
void heap_stats(uint32_t* total, uint32_t* used, uint32_t* free_blocks) {
    *total = 0;
    *used = 0;
    *free_blocks = 0;
    
    heap_block_t* current = heap_head;
    while (current != NULL) {
        *total += current->size + sizeof(heap_block_t);
        if (!current->is_free) {
            *used += current->size + sizeof(heap_block_t);
        } else {
            (*free_blocks)++;
        }
        current = current->next;
    }
}
