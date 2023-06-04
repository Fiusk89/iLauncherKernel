#include <idt.h>

idt_entry_t idt[256];
idtr_t idtr;

extern void _load_idt(uint32_t);
extern void isr_null();

void load_idt(idtr_t r)
{
    _load_idt((uint32_t)(&r));
}

void idt_set_entry(uint8_t num, void *base, uint16_t sel, uint8_t flags)
{
    idt[num].base_lo = ((uint32_t)base & 0xFFFF);
    idt[num].base_hi = ((uint32_t)base >> 16) & 0xFFFF;
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags | 0x60;
}

void idt_install()
{
    idtr.limit = (sizeof(idt_entry_t) * 256) - 1;
    idtr.base = (uint32_t)&idt;
    memset(idt, 0, sizeof(idt));
    load_idt(idtr);
}