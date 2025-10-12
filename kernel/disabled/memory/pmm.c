#include "pmm.h"
#include "serial.h"

// Bitmap to track free/used blocks
static uint32_t* memory_bitmap = 0;
static uint32_t total_blocks = 0;
static uint32_t used_blocks = 0;
static uint32_t bitmap_size = 0;

// Helper: Set bit in bitmap
static inline void bitmap_set(uint32_t bit) {
    memory_bitmap[bit / 32] |= (1 << (bit % 32));
}

// Helper: Clear bit in bitmap
static inline void bitmap_clear(uint32_t bit) {
    memory_bitmap[bit / 32] &= ~(1 << (bit % 32));
}

// Helper: Test bit in bitmap
static inline int bitmap_test(uint32_t bit) {
    return memory_bitmap[bit / 32] & (1 << (bit % 32));
}

// Find first free block
static int bitmap_find_first_free(void) {
    for (uint32_t i = 0; i < total_blocks; i++) {
        if (!bitmap_test(i)) {
            return i;
        }
    }
    return -1;
}

void pmm_init(uint32_t mem_low, uint32_t mem_high) {
    serial_write("PMM: Initializing Physical Memory Manager...\n");

    // Calculate total memory in bytes
    uint32_t total_memory = mem_low + mem_high;

    // Calculate number of blocks (4KB each)
    total_blocks = total_memory / PAGE_SIZE;
    used_blocks = total_blocks; // Initially mark all as used

    // Calculate bitmap size in bytes (1 bit per block)
    bitmap_size = total_blocks / 8;
    if (bitmap_size * 8 < total_blocks) {
        bitmap_size++;
    }

    // Place bitmap after kernel (at 2MB mark to be safe)
    memory_bitmap = (uint32_t*)0x200000;

    // Initialize bitmap - mark all as used
    for (uint32_t i = 0; i < bitmap_size / 4; i++) {
        memory_bitmap[i] = 0xFFFFFFFF;
    }

    serial_write("PMM: Total memory = ");
    char buf[32];
    int idx = 0;
    uint32_t mb = total_memory / (1024 * 1024);
    if (mb == 0) {
        buf[idx++] = '0';
    } else {
        char temp[32];
        int j = 0;
        while (mb > 0) {
            temp[j++] = '0' + (mb % 10);
            mb /= 10;
        }
        while (j > 0) {
            buf[idx++] = temp[--j];
        }
    }
    buf[idx] = '\0';
    serial_write(buf);
    serial_write(" MB\n");

    serial_write("PMM: Total blocks = ");
    idx = 0;
    uint32_t n = total_blocks;
    if (n == 0) {
        buf[idx++] = '0';
    } else {
        char temp[32];
        int j = 0;
        while (n > 0) {
            temp[j++] = '0' + (n % 10);
            n /= 10;
        }
        while (j > 0) {
            buf[idx++] = temp[--j];
        }
    }
    buf[idx] = '\0';
    serial_write(buf);
    serial_write("\n");

    serial_write("PMM: Bitmap size = ");
    idx = 0;
    n = bitmap_size;
    if (n == 0) {
        buf[idx++] = '0';
    } else {
        char temp[32];
        int j = 0;
        while (n > 0) {
            temp[j++] = '0' + (n % 10);
            n /= 10;
        }
        while (j > 0) {
            buf[idx++] = temp[--j];
        }
    }
    buf[idx] = '\0';
    serial_write(buf);
    serial_write(" bytes\n");

    serial_write("PMM: Initialized successfully\n");
}

// Mark a region of memory as free
void pmm_init_region(uint32_t addr, uint32_t size) {
    uint32_t start_block = addr / PAGE_SIZE;
    uint32_t num_blocks = size / PAGE_SIZE;

    for (uint32_t i = 0; i < num_blocks; i++) {
        bitmap_clear(start_block + i);
        used_blocks--;
    }
}

// Mark a region of memory as used
void pmm_deinit_region(uint32_t addr, uint32_t size) {
    uint32_t start_block = addr / PAGE_SIZE;
    uint32_t num_blocks = size / PAGE_SIZE;

    for (uint32_t i = 0; i < num_blocks; i++) {
        bitmap_set(start_block + i);
        used_blocks++;
    }
}

// Allocate a physical memory block (4KB page)
void* pmm_alloc_block(void) {
    if (used_blocks >= total_blocks) {
        return 0; // Out of memory
    }

    int block = bitmap_find_first_free();
    if (block == -1) {
        return 0; // No free blocks
    }

    bitmap_set(block);
    used_blocks++;

    uint32_t addr = block * PAGE_SIZE;
    return (void*)addr;
}

// Free a physical memory block
void pmm_free_block(void* addr) {
    uint32_t block = (uint32_t)addr / PAGE_SIZE;
    bitmap_clear(block);
    used_blocks--;
}

// Get total memory in bytes
uint32_t pmm_get_total_memory(void) {
    return total_blocks * PAGE_SIZE;
}

// Get number of used blocks
uint32_t pmm_get_used_blocks(void) {
    return used_blocks;
}

// Get number of free blocks
uint32_t pmm_get_free_blocks(void) {
    return total_blocks - used_blocks;
}

// Get total memory size in bytes
uint32_t pmm_get_memory_size(void) {
    return pmm_get_total_memory();
}
