#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

// Initialize keyboard
void keyboard_init(void);

// Get last key pressed (blocking)
char keyboard_getchar(void);

// Check if key is available
int keyboard_available(void);

#endif // KEYBOARD_H
