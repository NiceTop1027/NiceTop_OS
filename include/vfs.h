#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_FILENAME 32
#define MAX_FILES 64
#define MAX_FILE_SIZE 4096

// File types
typedef enum {
    FILE_TYPE_REGULAR,
    FILE_TYPE_DIRECTORY
} file_type_t;

// File structure
typedef struct {
    char name[MAX_FILENAME];
    file_type_t type;
    uint32_t size;
    char* data;
    bool in_use;
} file_t;

// VFS functions
void vfs_init(void);
file_t* vfs_create(const char* name, file_type_t type);
file_t* vfs_open(const char* name);
int vfs_write(file_t* file, const char* data, uint32_t size);
int vfs_read(file_t* file, char* buffer, uint32_t size);
int vfs_delete(const char* name);
void vfs_list(void);
int vfs_get_file_count(void);
file_t* vfs_get_file(int index);

#endif // VFS_H
