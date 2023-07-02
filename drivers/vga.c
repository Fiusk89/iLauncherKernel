#include <vga.h>

screen_info_t *vga_modes;

void vga_install()
{
    for (uint8_t i = 0; i < 255; i++)
        vga_palette[i] = (i << 16) |
                         (i << 8) |
                         i;
}

void vga_set_mode(uint8_t *regs)
{
    outb(VGA_MISC_WRITE, *regs++);
    for (size_t i = 0; i < VGA_NUM_SEQ_REGS; i++)
    {
        outb(VGA_SEQ_INDEX, i);
        outb(VGA_SEQ_DATA, *regs++);
    }
    regs[0x03] |= 0x80, regs[0x11] &= ~0x80;
    outb(VGA_CRTC_INDEX, 0x03);
    outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) | 0x80);
    outb(VGA_CRTC_INDEX, 0x11);
    outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) & ~0x80);
    for (size_t i = 0; i < VGA_NUM_CRTC_REGS; i++)
    {
        outb(VGA_CRTC_INDEX, i);
        outb(VGA_CRTC_DATA, *regs++);
    }
    for (size_t i = 0; i < VGA_NUM_GC_REGS; i++)
    {
        outb(VGA_GC_INDEX, i);
        outb(VGA_GC_DATA, *regs++);
    }
    for (size_t i = 0; i < VGA_NUM_AC_REGS; i++)
    {
        inb(VGA_INSTAT_READ);
        outb(VGA_AC_INDEX, i);
        outb(VGA_AC_WRITE, *regs++);
    }
    inb(VGA_INSTAT_READ);
    outb(VGA_AC_INDEX, 0x20);
}

void vga_set_color(uint8_t index, uint32_t rgb)
{
    outb(VGA_DAC_WRITE_INDEX, index);
    outb(VGA_DAC_DATA, (rgb & 0xff));
    outb(VGA_DAC_DATA, ((rgb >> 8) & 0xff));
    outb(VGA_DAC_DATA, ((rgb >> 16) & 0xff));
}

void vga_set_palette(uint32_t *palette)
{
    for (uint8_t i = 0; i < 255; i++)
        vga_set_color(i, palette[i]);
}

void vga_copy2framebuffer(uint8_t *vga_buffer, uint8_t p)
{
    uint8_t plane = (1 << p);
    uint8_t *vga_memory = (uint8_t *)0xA0000;
    outb(VGA_SEQ_INDEX, 0x02);
    outb(VGA_SEQ_DATA, plane);
    for (uint16_t i = 0; i < 38400; i++)
    {
        uint8_t bitmask = (!!(*vga_buffer++ & plane) << 7) |
                          (!!(*vga_buffer++ & plane) << 6) |
                          (!!(*vga_buffer++ & plane) << 5) |
                          (!!(*vga_buffer++ & plane) << 4) |
                          (!!(*vga_buffer++ & plane) << 3) |
                          (!!(*vga_buffer++ & plane) << 2) |
                          (!!(*vga_buffer++ & plane) << 1) |
                          (!!(*vga_buffer++ & plane));
        *vga_memory++ = bitmask;
    }
}