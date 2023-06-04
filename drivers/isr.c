#include <isr.h>

handler_t isr_handlers[256] = {0};

void isr_add_handler(uint8_t isr, handler_t handler)
{
    isr_handlers[isr] = handler;
}

void isr_remove_handler(uint8_t isr)
{
    isr_handlers[isr] = 0;
}

void isr_handler(register_t regs)
{
    if (isr_handlers[regs.int_no])
    {
        handler_t handler = isr_handlers[regs.int_no];
        handler(&regs);
    }
}

void isr_install()
{
    idt_set_entry(0, isr0, 0x08, 0x8E);
    idt_set_entry(1, isr1, 0x08, 0x8E);
    idt_set_entry(2, isr2, 0x08, 0x8E);
    idt_set_entry(3, isr3, 0x08, 0x8E);
    idt_set_entry(4, isr4, 0x08, 0x8E);
    idt_set_entry(5, isr5, 0x08, 0x8E);
    idt_set_entry(6, isr6, 0x08, 0x8E);
    idt_set_entry(7, isr7, 0x08, 0x8E);
    idt_set_entry(8, isr8, 0x08, 0x8E);
    idt_set_entry(9, isr9, 0x08, 0x8E);
    idt_set_entry(10, isr10, 0x08, 0x8E);
    idt_set_entry(11, isr11, 0x08, 0x8E);
    idt_set_entry(12, isr12, 0x08, 0x8E);
    idt_set_entry(13, isr13, 0x08, 0x8E);
    idt_set_entry(14, isr14, 0x08, 0x8E);
    idt_set_entry(15, isr15, 0x08, 0x8E);
    idt_set_entry(16, isr16, 0x08, 0x8E);
    idt_set_entry(17, isr17, 0x08, 0x8E);
    idt_set_entry(18, isr18, 0x08, 0x8E);
    idt_set_entry(19, isr19, 0x08, 0x8E);
    idt_set_entry(20, isr20, 0x08, 0x8E);
    idt_set_entry(21, isr21, 0x08, 0x8E);
    idt_set_entry(22, isr22, 0x08, 0x8E);
    idt_set_entry(23, isr23, 0x08, 0x8E);
    idt_set_entry(24, isr24, 0x08, 0x8E);
    idt_set_entry(25, isr25, 0x08, 0x8E);
    idt_set_entry(26, isr26, 0x08, 0x8E);
    idt_set_entry(27, isr27, 0x08, 0x8E);
    idt_set_entry(28, isr28, 0x08, 0x8E);
    idt_set_entry(29, isr29, 0x08, 0x8E);
    idt_set_entry(30, isr30, 0x08, 0x8E);
    idt_set_entry(31, isr31, 0x08, 0x8E);
    idt_set_entry(128, isr128, 0x08, 0x8E);
}