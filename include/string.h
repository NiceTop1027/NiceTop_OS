#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdarg.h>

// String functions
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
size_t strlen(const char* str);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
void* memset(void* ptr, int value, size_t num);
void* memcpy(void* dest, const void* src, size_t n);

// Formatted output
int sprintf(char* str, const char* format, ...);
int vsprintf(char* str, const char* format, va_list ap);

#endif // STRING_H
