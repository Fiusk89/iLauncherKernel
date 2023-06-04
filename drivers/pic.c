#include <pic.h>

uint16_t pic_cache_irq_mask = 0xffff & ~(1 << PIC_CASCADE);

uint16_t pic_read_irq_register(uint8_t ocw3)
{
    uint16_t value = 0;
    outb(PIC1_COMMAND, ocw3);
    outb(PIC2_COMMAND, ocw3);
    value = (inb(PIC2_COMMAND) << 8) | inb(PIC1_COMMAND);
    outb(PIC1_COMMAND, PIC_READ_IRR);
    outb(PIC2_COMMAND, PIC_READ_IRR);
    return value;
}

void pic_mask_irq(uint8_t irq)
{
    if (irq > 15 || irq == PIC_CASCADE || irq < 0)
        return;
    uint8_t port = irq < 8 ? PIC1_DATA : PIC2_DATA;
    pic_cache_irq_mask |= (1 << irq);
    outb(port, irq < 8 ? (pic_cache_irq_mask & 0xff)
                       : ((pic_cache_irq_mask >> 8) & 0xff));
}

void pic_unmask_irq(uint8_t irq)
{
    if (irq > 15 || irq < 0)
        return;
    uint8_t port = irq < 8 ? PIC1_DATA : PIC2_DATA;
    pic_cache_irq_mask &= ~(1 << irq);
    outb(port, irq < 8 ? (pic_cache_irq_mask & 0xff)
                       : ((pic_cache_irq_mask >> 8) & 0xff));
}

void pic_send_eoi(uint8_t irq, uint8_t mask)
{
    if (irq > 15 || irq < 0)
        return;
    if (irq < 8)
    {
        if (mask && irq != PIC_CASCADE)
            outb(PIC1_DATA, (irq < 8 ? (pic_cache_irq_mask & 0xff)
                                     : ((pic_cache_irq_mask >> 8) & 0xff)) |
                                (1 << irq));
        outb(PIC1_COMMAND, PIC_SPEC_EOI | irq);
    }
    else
    {
        if (mask)
            outb(PIC2_DATA, (irq < 8 ? (pic_cache_irq_mask & 0xff)
                                     : ((pic_cache_irq_mask >> 8) & 0xff)) |
                                (1 << (irq - 8)));
        outb(PIC2_COMMAND, PIC_SPEC_EOI | (irq - 8));
        outb(PIC1_COMMAND, PIC_SPEC_EOI | PIC_CASCADE);
    }
}

void pic_install()
{
    outb(PIC1_DATA, 0xff);
    io_wait();
    outb(PIC2_DATA, 0xff);
    io_wait();
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC1_DATA, 0x20);
    io_wait();
    outb(PIC2_DATA, 0x28);
    io_wait();
    outb(PIC1_DATA, 0x04);
    io_wait();
    outb(PIC2_DATA, 0x02);
    io_wait();
    outb(PIC1_DATA, pic_cache_irq_mask & 0xff);
    io_wait();
    outb(PIC2_DATA, (pic_cache_irq_mask >> 8) & 0xff);
    io_wait();
}

void pic_unmask_all()
{
    pic_cache_irq_mask = 0x0000;
    outb(PIC1_DATA, 0x00);
    outb(PIC2_DATA, 0x00);
}

void pic_mask_all()
{
    pic_cache_irq_mask = 0xffff;
    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);
}
