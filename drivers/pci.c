#include <pci.h>

uint16_t pci_read(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset)
{
    outl(0xcf8, (bus << 16) | (slot << 11) | (function << 8) | (offset & 0xfc) | 0x80000000);
    return ((uint16_t)((inl(0xcfc) >> ((offset & 2) << 3)) & 0xffff));
}

void pci_write(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint32_t value)
{
    outl(0xcf8, (bus << 16) | (slot << 11) | (function << 8) | (offset & 0xfc) | 0x80000000);
    outl(0xcfc, value);
}

uint32_t pci_read_bar_address(uint8_t bus, uint8_t slot, uint8_t function, uint8_t bar)
{
    return ((pci_read(bus, slot, function, bar + 2) << 16) | (pci_read(bus, slot, function, bar) & 0xffff));
}

void pci_write_bar_address(uint8_t bus, uint8_t slot, uint8_t function, uint8_t bar, uint32_t value)
{
    pci_write(bus, slot, function, bar, value & 0xffff);
    pci_write(bus, slot, function, bar + 2, (value >> 16) & 0xffff);
}

uint32_t *pci_find_devices(uint8_t length, uint8_t class, uint8_t subclass, uint8_t subsubclass)
{
    uint32_t *devices = (uint32_t *)kmalloc(256 * sizeof(uint32_t));
    uint32_t devices_pos = 0;
    memset(devices, 0, 256 * sizeof(uint32_t));
    if (!devices)
        return (void*)NULL;
    for (uint8_t bus = 0; bus < 256; bus++)
    {
        for (uint8_t slot = 0; slot < 32; slot++)
        {
            for (uint8_t function = 0; function < 8; function++)
            {
                uint16_t vendor = pci_read(bus, slot, function, 0x00);
                uint16_t device = pci_read(bus, slot, function, 0x02);
                if (vendor != 0xffff)
                {
                    uint8_t pci_class = (pci_read(bus, slot, function, 0x0A) >> 8) & 0xFF;
                    uint8_t pci_subclass = (pci_read(bus, slot, function, 0x0A)) & 0xFF;
                    uint8_t pci_subsubclass = (pci_read(bus, slot, function, 0x08) >> 8) & 0xFF;
                    if (length == 1)
                    {
                        if (pci_class == class)
                            devices[devices_pos++] = (bus << 16) |
                                                     (slot << 8) |
                                                     function;
                    }
                    else if (length == 2)
                    {
                        if (pci_class == class && pci_subclass == subclass)
                            devices[devices_pos++] = (bus << 16) |
                                                     (slot << 8) |
                                                     function;
                    }
                    else if (length == 3)
                    {
                        if (pci_class == class && pci_subclass == subclass && pci_subsubclass == subsubclass)
                            devices[devices_pos++] = (bus << 16) |
                                                     (slot << 8) |
                                                     function;
                    }
                }
            }
        }
    }
    return devices;
}