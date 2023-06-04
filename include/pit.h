#ifndef PIT_H
#define PIT_H
#include <kernel.h>
#define PIT_FREQUENCY 1193180
#define PIT_HZ 1000
typedef uint32_t pit_t;
extern pit_t pit_ticks;

void pit_install();
#endif