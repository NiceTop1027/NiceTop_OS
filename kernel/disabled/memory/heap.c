#include "heap.h"
#include "pmm.h"
#include "paging.h"
#include "serial.h"

// Heap block header
typedef struct heap_block {
    size_t size;
    int is_free;
    struct heap_block* next;
} heap_block_t;

// Heap start and end
static uint32_t heap_start = 0;
static uint32_t heap_end = 0;
static heap_block_t* heap_head = 0;

#define HEAP_START 0x400000  // Start at 4MB
#define HEAP_INITIAL_SIZE 0x100000  // Initial 1MB

// Align address to page boundary
static uint32_t align_page(uint32_t addr) {
    if (addr & 0xFFF) {
        addr = (addr & 0xFFFFF000) + 0x1000;
    }
    return addr;
}

void heap_init(void) {
    serial_write("Heap: Initializing...\n");

    heap_start = HEAP_START;
    heap_end = heap_start + HEAP_INITIAL_SIZE;

    // Map heap pages
    page_directory_t* dir = paging_get_directory();
    for (uint32_t addr = heap_start; addr < heap_end; addr += PAGE_SIZE) {
        void* frame = pmm_alloc_block();
        paging_map_page(dir, addr, (uint32_t)frame, PAGE_PRESENT | PAGE_WRITE);
    }

    // Initialize first block
    heap_head = (heap_block_t*)heap_start;
    heap_head->size = HEAP_INITIAL_SIZE - sizeof(heap_block_t);
    heap_head->is_free = 1;
    heap_head->next = 0;

    serial_write("Heap: Initialized at 0x");
    char hex[9];
    for (int i = 7; i >= 0; i--) {
        hex[7-i] = "0123456789ABCDEF"[(heap_start >> (i*4)) & 0xF];
    }
    hex[8] = '\0';
    serial_write(hex);
    serial_write("\n");
}

// Find free block (first-fit algorithm)
static heap_block_t* find_free_block(heap_block_t** last, size_t size) {
    heap_block_t* current = heap_head;
    while (current && !(current->is_free && current->size >= size)) {
        *last = current;
        current = current->next;
    }
    return current;
}

// Expand heap if needed
static heap_block_t* expand_heap(heap_block_t* last, size_t size) {
    heap_block_t* block;
    uint32_t old_end = heap_end;

    // Calculate new heap end
    heap_end += size + sizeof(heap_block_t);
    heap_end = align_page(heap_end);

    // Map new pages
    page_directory_t* dir = paging_get_directory();
    for (uint32_t addr = old_end; addr < heap_end; addr += PAGE_SIZE) {
        void* frame = pmm_alloc_block();
        paging_map_page(dir, addr, (uint32_t)frame, PAGE_PRESENT | PAGE_WRITE);
    }

    block = (heap_block_t*)old_end;
    block->size = size;
    block->is_free = 0;
    block->next = 0;

    if (last) {
        last->next = block;
    }

    return block;
}

void* kmalloc(size_t size) {
    if (size == 0) {
        return 0;
    }

    if (!heap_head) {
        heap_init();
    }

    heap_block_t* last = 0;
    heap_block_t* block = find_free_block(&last, size);

    if (!block) {
        // No free block found, expand heap
        block = expand_heap(last, size);
        if (!block) {
            return 0;
        }
    } else {
        // Found a free block
        block->is_free = 0;

        // Split block if too large
        if (block->size > size + sizeof(heap_block_t)) {
            heap_block_t* new_block = (heap_block_t*)((uint32_t)block + sizeof(heap_block_t) + size);
            new_block->size = block->size - size - sizeof(heap_block_t);
            new_block->is_free = 1;
            new_block->next = block->next;

            block->size = size;
            block->next = new_block;
        }
    }

    return (void*)((uint32_t)block + sizeof(heap_block_t));
}

void* kmalloc_a(size_t size) {
    // Allocate page-aligned memory
    uint32_t addr = (uint32_t)kmalloc(size);
    if (addr & 0xFFF) {
        // Not aligned, allocate more
        uint32_t aligned = align_page(addr);
        return (void*)aligned;
    }
    return (void*)addr;
}

void* kmalloc_p(size_t size, uint32_t* phys) {
    void* addr = kmalloc(size);
    if (phys) {
        // For now, just return virtual address (identity mapped)
        *phys = (uint32_t)addr;
    }
    return addr;
}

void* kmalloc_ap(size_t size, uint32_t* phys) {
    void* addr = kmalloc_a(size);
    if (phys) {
        *phys = (uint32_t)addr;
    }
    return addr;
}

void kfree(void* ptr) {
    if (!ptr) {
        return;
    }

    // Get block header
    heap_block_t* block = (heap_block_t*)((uint32_t)ptr - sizeof(heap_block_t));
    block->is_free = 1;

    // Merge with next block if free
    if (block->next && block->next->is_free) {
        block->size += sizeof(heap_block_t) + block->next->size;
        block->next = block->next->next;
    }

    // Merge with previous block if free
    heap_block_t* current = heap_head;
    while (current && current->next != block) {
        current = current->next;
    }

    if (current && current->is_free) {
        current->size += sizeof(heap_block_t) + block->size;
        current->next = block->next;
    }
}
