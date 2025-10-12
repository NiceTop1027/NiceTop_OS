#include "vfs.h"
#include "heap.h"
#include "serial.h"
#include <stddef.h>

static file_t files[MAX_FILES];

// String functions
static int str_len(const char* str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

static void str_copy(char* dest, const char* src, int max) {
    int i = 0;
    while (src[i] && i < max - 1) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

static int str_cmp(const char* s1, const char* s2) {
    int i = 0;
    while (s1[i] && s2[i]) {
        if (s1[i] != s2[i]) return s1[i] - s2[i];
        i++;
    }
    return s1[i] - s2[i];
}

void vfs_init(void) {
    serial_write("VFS: Initializing...\n");
    
    for (int i = 0; i < MAX_FILES; i++) {
        files[i].in_use = false;
        files[i].data = NULL;
        files[i].size = 0;
    }
    
    // Create some default files
    file_t* readme = vfs_create("README.txt", FILE_TYPE_REGULAR);
    if (readme) {
        const char* content = "Welcome to NiceTop OS!\n\nThis is a simple operating system built from scratch.\nType 'help' to see available commands.\n";
        vfs_write(readme, content, str_len(content));
    }
    
    file_t* welcome = vfs_create("welcome.txt", FILE_TYPE_REGULAR);
    if (welcome) {
        const char* content = "Hello, World!\nNiceTop OS is running!\n";
        vfs_write(welcome, content, str_len(content));
    }
    
    serial_write("VFS: Initialized with default files\n");
}

file_t* vfs_create(const char* name, file_type_t type) {
    // Check if file already exists
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].in_use && str_cmp(files[i].name, name) == 0) {
            return NULL; // File already exists
        }
    }
    
    // Find free slot
    for (int i = 0; i < MAX_FILES; i++) {
        if (!files[i].in_use) {
            files[i].in_use = true;
            str_copy(files[i].name, name, MAX_FILENAME);
            files[i].type = type;
            files[i].size = 0;
            files[i].data = NULL;
            return &files[i];
        }
    }
    
    return NULL; // No free slots
}

file_t* vfs_open(const char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].in_use && str_cmp(files[i].name, name) == 0) {
            return &files[i];
        }
    }
    return NULL;
}

int vfs_write(file_t* file, const char* data, uint32_t size) {
    if (!file || size > MAX_FILE_SIZE) return -1;
    
    // Allocate or reallocate data
    if (file->data) {
        kfree(file->data);
    }
    
    file->data = (char*)kmalloc(size + 1);
    if (!file->data) return -1;
    
    // Copy data
    for (uint32_t i = 0; i < size; i++) {
        file->data[i] = data[i];
    }
    file->data[size] = '\0';
    file->size = size;
    
    return size;
}

int vfs_read(file_t* file, char* buffer, uint32_t size) {
    if (!file || !file->data) return -1;
    
    uint32_t read_size = (size < file->size) ? size : file->size;
    
    for (uint32_t i = 0; i < read_size; i++) {
        buffer[i] = file->data[i];
    }
    buffer[read_size] = '\0';
    
    return read_size;
}

int vfs_delete(const char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].in_use && str_cmp(files[i].name, name) == 0) {
            if (files[i].data) {
                kfree(files[i].data);
            }
            files[i].in_use = false;
            files[i].data = NULL;
            files[i].size = 0;
            return 0;
        }
    }
    return -1;
}

void vfs_list(void) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].in_use) {
            serial_write("  ");
            serial_write(files[i].name);
            if (files[i].type == FILE_TYPE_DIRECTORY) {
                serial_write("/");
            }
            serial_write("\n");
        }
    }
}

int vfs_get_file_count(void) {
    int count = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].in_use) count++;
    }
    return count;
}

file_t* vfs_get_file(int index) {
    int count = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].in_use) {
            if (count == index) return &files[i];
            count++;
        }
    }
    return NULL;
}
