#include <fpu.h>

void fpu_install()
{
    if (!cpu.feature.edx.fpu)
        return;
    uint64_t cr4;
    asm volatile("mov %%cr4, %0"
                 : "=r"(cr4));
    cr4 |= 0x200;
    asm volatile("mov %0, %%cr4" ::"r"(cr4));
    uint16_t tmp = 0x37F;
    asm volatile("fldcw %0" ::"m"(tmp));
    asm volatile("fninit");
}