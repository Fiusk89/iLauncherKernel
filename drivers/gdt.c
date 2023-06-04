#include <gdt.h>

gdt_entry_t gdt[9];
gdtr_t gdtr;

extern void _load_gdt(uint32_t);
void load_gdt(gdtr_t r)
{
    _load_gdt((uint32_t)(&r));
}

void gdt_set_entry(uint8_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;
    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;
    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access = access;
}

void gdt_install()
{
    gdtr.limit = (sizeof(gdt_entry_t) * 9) - 1;
    gdtr.base = (uint32_t)&gdt;
    gdt_set_entry(0, 0, 0, 0, 0);
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);
    gdt_set_entry(6, 0, 0xFFFFFFFF, 0x9A, 0x0F);
    gdt_set_entry(7, 0, 0xFFFFFFFF, 0x92, 0x0F);
    load_gdt(gdtr);
    tss_install(8, 0x10, 0);
}