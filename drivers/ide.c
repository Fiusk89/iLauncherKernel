#include <ide.h>

struct IDEChannelRegisters
{
    uint16_t base;  // I/O Base.
    uint16_t ctrl;  // Control Base
    uint16_t bmide; // Bus Master IDE
    uint8_t nIEN;   // nIEN (No Interrupt);
} ide_channel[2];

struct ide_device
{
    uint8_t Reserved;      // 0 (Empty) or 1 (This Drive really exists).
    uint8_t Channel;       // 0 (Primary Channel) or 1 (Secondary Channel).
    uint8_t Drive;         // 0 (Master Drive) or 1 (Slave Drive).
    uint16_t Type;         // 0: ATA, 1:ATAPI.
    uint16_t Signature;    // Drive Signature
    uint16_t Capabilities; // Features.
    uint32_t CommandSets;  // Command Sets Supported.
    uint32_t Size;         // Size in Sectors.
    uint8_t Model[41];     // Model in string.
} ide_devices[4];

uint8_t ide_buf[2048] = {0};
uint8_t ide_irq_invoked = 0;
uint8_t atapi_packet[12] = {0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

uint8_t ide_read(uint8_t channel, uint8_t reg)
{
    uint8_t ret;
    if (reg > 0x07 && reg < 0x0c)
        ide_write(channel, ATA_REG_CONTROL, 0x80 | ide_channel[channel].nIEN);
    if (reg < 0x08)
        ret = inb(ide_channel[channel].base + reg);
    else if (reg < 0x0c)
        ret = inb(ide_channel[channel].base + reg - 0x06);
    else if (reg < 0x0e)
        ret = inb(ide_channel[channel].ctrl + reg - 0x0a);
    else if (reg < 0x16)
        ret = inb(ide_channel[channel].bmide + reg - 0x0e);
    if (reg > 0x07 && reg < 0x0c)
        ide_write(channel, ATA_REG_CONTROL, ide_channel[channel].nIEN);
    return ret;
}

void ide_write(uint8_t channel, uint8_t reg, uint8_t value)
{
    uint8_t ret;
    if (reg > 0x07 && reg < 0x0c)
        ide_write(channel, ATA_REG_CONTROL, 0x80 | ide_channel[channel].nIEN);
    if (reg < 0x08)
        outb(ide_channel[channel].base + reg, value);
    else if (reg < 0x0c)
        outb(ide_channel[channel].base + reg - 0x06, value);
    else if (reg < 0x0e)
        outb(ide_channel[channel].ctrl + reg - 0x0a, value);
    else if (reg < 0x16)
        outb(ide_channel[channel].bmide + reg - 0x0e, value);
    if (reg > 0x07 && reg < 0x0c)
        ide_write(channel, ATA_REG_CONTROL, ide_channel[channel].nIEN);
}

void ide_install()
{
}