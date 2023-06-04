#ifndef CPU_H
#define CPU_H
#include <kernel.h>

bool cpu_check_msr();
void cpu_read_msr(uint32_t msr, uint32_t *lo, uint32_t *hi);
void cpu_write_msr(uint32_t msr, uint32_t lo, uint32_t hi);
#endif