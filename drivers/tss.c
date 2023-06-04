#include <tss.h>

tss_entry_t kernel_tss;
extern void _load_tss();

void tss_install(uint32_t idx, uint32_t kss, uint32_t kesp)
{
    uint32_t base = (uint32_t)&kernel_tss;
    gdt_set_entry(idx, base, base + sizeof(tss_entry_t), 0x89, 0x40);
    memset(&kernel_tss, 0, sizeof(tss_entry_t));
    _load_tss();
}

void tss_set_stack(uint32_t kss, uint32_t kesp)
{
    kernel_tss.ss0 = kss;
    kernel_tss.esp0 = kesp;
}
