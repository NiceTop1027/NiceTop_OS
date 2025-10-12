#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

// Initialize timer
void timer_init(uint32_t frequency);

// Get tick count
uint32_t timer_get_ticks(void);

#endif // TIMER_H
