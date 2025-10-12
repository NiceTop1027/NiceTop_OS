#include "paging.h"
#include "pmm.h"
#include "serial.h"

// Current page directory
static page_directory_t* current_directory = 0;
static page_directory_t* kernel_directory = 0;

// Page fault handler
void page_fault_handler(void) {
    serial_write("PAGE FAULT!\n");
    while (1) {
        __asm__ volatile("hlt");
    }
}

// Get a page from page directory (create if doesn't exist)
static page_t* paging_get_page(page_directory_t* dir, uint32_t address, int make) {
    uint32_t table_idx = address / (PAGE_SIZE * PAGE_TABLE_SIZE);

    if (dir->tables[table_idx]) {
        uint32_t page_idx = (address / PAGE_SIZE) % PAGE_TABLE_SIZE;
        return &dir->tables[table_idx]->pages[page_idx];
    } else if (make) {
        uint32_t phys = 0;
        dir->tables[table_idx] = (page_table_t*)pmm_alloc_block();
        phys = (uint32_t)dir->tables[table_idx];

        // Clear the page table
        for (int i = 0; i < PAGE_TABLE_SIZE * sizeof(page_t) / 4; i++) {
            ((uint32_t*)dir->tables[table_idx])[i] = 0;
        }

        dir->tables_physical[table_idx] = phys | PAGE_PRESENT | PAGE_WRITE;

        uint32_t page_idx = (address / PAGE_SIZE) % PAGE_TABLE_SIZE;
        return &dir->tables[table_idx]->pages[page_idx];
    }

    return 0;
}

void paging_map_page(page_directory_t* dir, uint32_t virt, uint32_t phys, uint32_t flags) {
    page_t* page = paging_get_page(dir, virt, 1);
    if (page) {
        page->present = (flags & PAGE_PRESENT) ? 1 : 0;
        page->rw = (flags & PAGE_WRITE) ? 1 : 0;
        page->user = (flags & PAGE_USER) ? 1 : 0;
        page->frame = phys / PAGE_SIZE;
    }
}

void paging_unmap_page(page_directory_t* dir, uint32_t virt) {
    page_t* page = paging_get_page(dir, virt, 0);
    if (page) {
        page->present = 0;
    }
}

void paging_switch_directory(page_directory_t* dir) {
    current_directory = dir;
    __asm__ volatile("mov %0, %%cr3" :: "r"(dir->physical_addr));
}

page_directory_t* paging_get_directory(void) {
    return current_directory;
}

void paging_init(void) {
    serial_write("Paging: Initializing...\n");

    // Use a static page directory to avoid PMM issues
    static page_directory_t kdir __attribute__((aligned(4096)));
    kernel_directory = &kdir;

    // Clear page directory
    for (int i = 0; i < PAGE_DIRECTORY_SIZE; i++) {
        kernel_directory->tables[i] = 0;
        kernel_directory->tables_physical[i] = 0;
    }

    // Physical address is the address of tables_physical array
    kernel_directory->physical_addr = (uint32_t)kernel_directory->tables_physical;

    // Identity map first 8MB (kernel + heap space)
    serial_write("Paging: Identity mapping first 8MB...\n");
    for (uint32_t i = 0; i < 0x800000; i += PAGE_SIZE) {
        paging_map_page(kernel_directory, i, i, PAGE_PRESENT | PAGE_WRITE);
    }

    // Switch to kernel page directory
    serial_write("Paging: Switching to kernel directory...\n");
    current_directory = kernel_directory;
    __asm__ volatile("mov %0, %%cr3" :: "r"(kernel_directory->physical_addr));

    // Enable paging
    serial_write("Paging: Enabling paging...\n");
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // Set PG bit
    __asm__ volatile("mov %0, %%cr0" :: "r"(cr0));

    serial_write("Paging: Enabled successfully\n");
}
