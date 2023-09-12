#include <io.h>

void outb(uint16_t port, uint8_t data)
{
    asm volatile("outb %1, %0"
                 :
                 : "dN"(port), "a"(data));
}

uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile("inb %1, %0"
                 : "=a"(ret)
                 : "Nd"(port));
    return ret;
}

uint16_t inw(uint16_t port)
{
    uint16_t rv;
    asm volatile("inw %1, %0"
                 : "=a"(rv)
                 : "dN"(port));
    return rv;
}

void outw(uint16_t port, uint16_t data)
{
    asm volatile("outw %1, %0"
                 :
                 : "dN"(port), "a"(data));
}

uint32_t inl(uint16_t port)
{
    uint32_t rv;
    asm volatile("inl %%dx, %%eax"
                 : "=a"(rv)
                 : "dN"(port));
    return rv;
}

void outl(uint16_t port, uint32_t data)
{
    asm volatile("outl %%eax, %%dx"
                 :
                 : "dN"(port), "a"(data));
}

void insm(uint16_t port, uint32_t buffer, uint32_t size)
{
    asm volatile("rep insw"
                 : "+d"(buffer), "+c"(size) : "d"(port) : "memory");
}

void outsm(uint16_t port, uint32_t buffer, uint32_t size)
{
    asm volatile("rep outsw"
                 : "+d"(buffer), "+c"(size) : "d"(port) : "memory");
}

void io_wait()
{
    outb(0x80, 0);
}

void mm_outb(void *port, uint8_t data)
{
    *(volatile uint8_t *)(port) = data;
}

uint8_t mm_inb(void *port)
{
    return *(volatile uint8_t *)(port);
}

void mm_outw(void *port, uint16_t data)
{
    *(volatile uint16_t *)(port) = data;
}

uint16_t mm_inw(void *port)
{
    return *(volatile uint16_t *)(port);
}

void mm_outl(void *port, uint32_t data)
{
    *(volatile uint32_t *)(port) = data;
}

uint32_t mm_inl(void *port)
{
    return *(volatile uint32_t *)(port);
}