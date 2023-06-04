#include <irq.h>

handler_t irq_handlers[16] = {0};
uint32_t irq_spurious = 0;
extern uint16_t pic_cache_irq_mask;

void irq_add_handler(uint8_t irq, handler_t handler)
{
    if (irq > 15)
        return;
    irq_handlers[irq] = handler;
    pic_unmask_irq(irq);
}

void irq_remove_handler(uint8_t irq)
{
    if (irq > 15)
        return;
    irq_handlers[irq] = 0;
    pic_mask_irq(irq);
}

void irq_handler(register_t regs)
{
    if (regs.int_no > 255 || regs.int_no < 32)
        return;
    uint8_t irq = regs.int_no - 32;
    uint16_t irq_mask = 1 << irq;
    uint8_t irq_is_masked = pic_cache_irq_mask & irq_mask;
    uint8_t irq_is_fake = ~pic_read_irq_register(PIC_READ_ISR) & irq_mask;
    if (irq != 0 && irq_is_masked | irq_is_fake)
    {
        kprintf("FAKE %s IRQ[%u]!\n", irq > 7 ? "SLAVE" : "MASTER", irq);
        if (irq_is_fake)
        {
            if (irq > 7)
                pic_send_eoi(PIC_CASCADE, irq_is_masked);
        }
        else
        {
            pic_send_eoi(irq, irq_is_masked);
        }
        irq_spurious++;
        return;
    }
    if (irq_handlers[irq])
    {
        handler_t handler = irq_handlers[irq];
        handler(&regs);
    }
    pic_send_eoi(irq, irq_handlers[irq] == 0);
}

void irq_install()
{
    idt_set_entry(32, irq0, 0x08, 0x8E);
    idt_set_entry(33, irq1, 0x08, 0x8E);
    idt_set_entry(34, irq2, 0x08, 0x8E);
    idt_set_entry(35, irq3, 0x08, 0x8E);
    idt_set_entry(36, irq4, 0x08, 0x8E);
    idt_set_entry(37, irq5, 0x08, 0x8E);
    idt_set_entry(38, irq6, 0x08, 0x8E);
    idt_set_entry(39, irq7, 0x08, 0x8E);
    idt_set_entry(40, irq8, 0x08, 0x8E);
    idt_set_entry(41, irq9, 0x08, 0x8E);
    idt_set_entry(42, irq10, 0x08, 0x8E);
    idt_set_entry(43, irq11, 0x08, 0x8E);
    idt_set_entry(44, irq12, 0x08, 0x8E);
    idt_set_entry(45, irq13, 0x08, 0x8E);
    idt_set_entry(46, irq14, 0x08, 0x8E);
    idt_set_entry(47, irq15, 0x08, 0x8E);
    pic_install();
    asm volatile("sti");
}
