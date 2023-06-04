#include <cpu.h>

bool cpu_check_msr()
{
    return cpu.feature.edx.msr;
}

void cpu_read_msr(uint32_t msr, uint32_t *lo, uint32_t *hi)
{
    if (cpu.feature.edx.msr)
        asm volatile("rdmsr"
            : "=a"(*lo), "=d"(*hi)
            : "c"(msr));
}

void cpu_write_msr(uint32_t msr, uint32_t lo, uint32_t hi)
{
    if (cpu.feature.edx.msr)
        asm volatile("wrmsr"
            :
            : "a"(lo), "d"(hi), "c"(msr));
}