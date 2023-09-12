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
    uint8_t reserved;      // 0 (Empty) or 1 (This Drive really exists).
    uint8_t channel;       // 0 (Primary Channel) or 1 (Secondary Channel).
    uint8_t drive;         // 0 (Master Drive) or 1 (Slave Drive).
    uint16_t type;         // 0: ATA, 1:ATAPI.
    uint16_t signature;    // Drive Signature
    uint16_t capabilities; // Features.
    uint32_t command_sets; // Command Sets Supported.
    uint32_t size;         // Size in Sectors.
    uint8_t model[256];    // Model in string.
} ide_device[5];

uint8_t ide_buffer[4096] = {0};
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

void ide_read_buffer(uint8_t channel, uint8_t reg, void *buffer, uint32_t size)
{
    if (reg > 0x07 && reg < 0x0c)
        ide_write(channel, ATA_REG_CONTROL, 0x80 | ide_channel[channel].nIEN);
    if (reg < 0x08)
        insm(ide_channel[channel].base + reg, (uint32_t)buffer, size);
    else if (reg < 0x0c)
        insm(ide_channel[channel].base + reg - 0x06, (uint32_t)buffer, size);
    else if (reg < 0x0e)
        insm(ide_channel[channel].ctrl + reg - 0x0a, (uint32_t)buffer, size);
    else if (reg < 0x16)
        insm(ide_channel[channel].bmide + reg - 0x0e, (uint32_t)buffer, size);
    if (reg > 0x07 && reg < 0x0c)
        ide_write(channel, ATA_REG_CONTROL, ide_channel[channel].nIEN);
}

void ide_write_buffer(uint8_t channel, uint8_t reg, void *buffer, uint32_t size)
{
    if (reg > 0x07 && reg < 0x0c)
        ide_write(channel, ATA_REG_CONTROL, 0x80 | ide_channel[channel].nIEN);
    if (reg < 0x08)
        outsm(ide_channel[channel].base + reg, (uint32_t)buffer, size);
    else if (reg < 0x0c)
        outsm(ide_channel[channel].base + reg - 0x06, (uint32_t)buffer, size);
    else if (reg < 0x0e)
        outsm(ide_channel[channel].ctrl + reg - 0x0a, (uint32_t)buffer, size);
    else if (reg < 0x16)
        outsm(ide_channel[channel].bmide + reg - 0x0e, (uint32_t)buffer, size);
    if (reg > 0x07 && reg < 0x0c)
        ide_write(channel, ATA_REG_CONTROL, ide_channel[channel].nIEN);
}

uint8_t ide_polling(uint8_t channel, uint8_t advanced_check)
{
    for (uint8_t i = 0; i < 4; i++)
        ide_read(channel, ATA_REG_ALTSTATUS);
    while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY)
        ;
    if (advanced_check)
    {
        uint8_t value = ide_read(channel, ATA_REG_STATUS);
        if (value & ATA_SR_ERR)
            return 2;
        if (value & ATA_SR_DF)
            return 1;
        if (value & ATA_SR_DRQ)
            return 3;
    }
    return 0;
}

void ide_install()
{
    uint32_t *ide_controllers = pci_find_devices(2, 0x01, 0x01, 0x00);
    if (!ide_controllers)
        return;
    uint8_t ide_count = 0;
    uint32_t ide_bar[] = {
        pci_read_bar_address(ide_controllers[0] & 0xff,
                             (ide_controllers[0] >> 8) & 0xff,
                             (ide_controllers[0] >> 16) & 0xff,
                             0),
        pci_read_bar_address(ide_controllers[0] & 0xff,
                             (ide_controllers[0] >> 8) & 0xff,
                             (ide_controllers[0] >> 16) & 0xff,
                             1),
        pci_read_bar_address(ide_controllers[0] & 0xff,
                             (ide_controllers[0] >> 8) & 0xff,
                             (ide_controllers[0] >> 16) & 0xff,
                             2),
        pci_read_bar_address(ide_controllers[0] & 0xff,
                             (ide_controllers[0] >> 8) & 0xff,
                             (ide_controllers[0] >> 16) & 0xff,
                             3),
        pci_read_bar_address(ide_controllers[0] & 0xff,
                             (ide_controllers[0] >> 8) & 0xff,
                             (ide_controllers[0] >> 16) & 0xff,
                             4),
    };
    ide_channel[ATA_PRIMARY].base = (ide_bar[0] & 0xfffffffc) + 0x1f0 * (!ide_bar[0]);
    ide_channel[ATA_PRIMARY].ctrl = (ide_bar[1] & 0xfffffffc) + 0x3f6 * (!ide_bar[1]);
    ide_channel[ATA_SECONDARY].base = (ide_bar[2] & 0xfffffffc) + 0x170 * (!ide_bar[2]);
    ide_channel[ATA_SECONDARY].ctrl = (ide_bar[3] & 0xfffffffc) + 0x376 * (!ide_bar[3]);
    ide_channel[ATA_PRIMARY].bmide = (ide_bar[4] & 0xfffffffc) + 0;
    ide_channel[ATA_SECONDARY].bmide = (ide_bar[4] & 0xfffffffc) + 8;
    ide_write(ATA_PRIMARY, ATA_REG_CONTROL, 1 << 1);
    ide_write(ATA_SECONDARY, ATA_REG_CONTROL, 1 << 1);
    for (uint8_t i = 0; i < 2; i++)
    {
        for (uint8_t j = 0; j < 2; j++)
        {
            uint8_t error = 0, type = IDE_ATA, status = 0;
            ide_device[ide_count].reserved = 0;
            ide_write(i, ATA_REG_HDDEVSEL, 0xa0 | (j << 4));
            ide_polling(i, false);
            ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
            ide_polling(i, false);
            if (!ide_read(i, ATA_REG_STATUS))
                continue;
            while (true)
            {
                status = ide_read(i, ATA_REG_STATUS);
                if (status & ATA_SR_ERR)
                {
                    error = 1;
                    break;
                }
                if (~status & ATA_SR_BSY && status & ATA_SR_DRQ)
                {
                    break;
                }
            }
            if (error)
            {
                uint8_t cl = ide_read(i, ATA_REG_LBA1);
                uint8_t ch = ide_read(i, ATA_REG_LBA2);
                if (cl == 0x14 && ch == 0xEB)
                    type = IDE_ATAPI;
                else if (cl == 0x69 && ch == 0x96)
                    type = IDE_ATAPI;
                else
                    continue;
                ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
                ide_polling(i, false);
            }
            ide_read_buffer(i, ATA_REG_DATA, (void *)ide_buffer, 128);
            ide_device[ide_count].reserved = 1;
            ide_device[ide_count].type = type;
            ide_device[ide_count].channel = i;
            ide_device[ide_count].drive = j;
            ide_device[ide_count].signature = *(uint16_t *)(&ide_buffer[ATA_IDENT_DEVICETYPE]);
            ide_device[ide_count].capabilities = *(uint16_t *)(&ide_buffer[ATA_IDENT_CAPABILITIES]);
            ide_device[ide_count].command_sets = *(uint32_t *)(&ide_buffer[ATA_IDENT_COMMANDSETS]);
            if (ide_device[ide_count].command_sets & (1 << 26))
                ide_device[ide_count].size = *(uint32_t *)(&ide_buffer[ATA_IDENT_MAX_LBA_EXT]);
            else
                ide_device[ide_count].size = *(uint32_t *)(&ide_buffer[ATA_IDENT_MAX_LBA]);
            for (uint8_t k = 0; k < 40; k++)
            {
                ide_device[ide_count].model[k] = ide_buffer[ATA_IDENT_MODEL + k + 1];
                ide_device[ide_count].model[k + 1] = ide_buffer[ATA_IDENT_MODEL + k];
            }
            ide_device[ide_count].model[40] = '\0';
            ide_count++;
        }
    }
    for (uint8_t i = 0; i < 4; i++)
    {
        if (ide_device[i].reserved)
        {
            kprintf("IDE: found %s, drive %uGB, model %s\n",
                    (const char *[]){"ATA", "ATAPI"}[ide_device[i].type],
                    ide_device[i].size / 1024 / 1024 / 2,
                    ide_device[i].model);
        }
    }
    while (true)
        ;
}