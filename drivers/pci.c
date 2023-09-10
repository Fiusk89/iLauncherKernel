#include <pci.h>

pci_t *pci;

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

uint32_t *pci_find_devices(uint8_t length, uint8_t class, uint8_t subclass, uint8_t interface)
{
    uint32_t *devices = (uint32_t *)kmalloc(sizeof(uint32_t));
    if (!devices)
        return (void *)NULL;
    uint8_t devices_pos = 0;
    memset(devices, 0, sizeof(uint32_t));
    pci_t *tmp_pci = pci;
    while (tmp_pci)
    {
        if (tmp_pci->vendor != 0xffff)
        {
            uint8_t old_devices_pos = devices_pos;
            switch (length & 3)
            {
            case 0 ... 1:
                if (tmp_pci->class == class)
                {
                    devices[devices_pos++] = (tmp_pci->function << 16) |
                                             (tmp_pci->slot << 8) |
                                             tmp_pci->bus;
                    kprintf("PCI: bus: %x, slot: %x, function: %x\n",
                            (uint32_t)tmp_pci->bus,
                            (uint32_t)tmp_pci->slot,
                            (uint32_t)tmp_pci->function);
                }
                break;
            case 2:
                if (tmp_pci->class == class &&
                    tmp_pci->subclass == subclass)
                {
                    devices[devices_pos++] = (tmp_pci->function << 16) |
                                             (tmp_pci->slot << 8) |
                                             tmp_pci->bus;
                    kprintf("PCI: bus: %x, slot: %x, function: %x\n",
                            (uint32_t)tmp_pci->bus,
                            (uint32_t)tmp_pci->slot,
                            (uint32_t)tmp_pci->function);
                }
                break;
            case 3:
                if (tmp_pci->class == class &&
                    tmp_pci->subclass == subclass &&
                    tmp_pci->interface == interface)
                {
                    devices[devices_pos++] = (tmp_pci->function << 16) |
                                             (tmp_pci->slot << 8) |
                                             tmp_pci->bus;
                    kprintf("PCI: bus: %x, slot: %x, function: %x\n",
                            (uint32_t)tmp_pci->bus,
                            (uint32_t)tmp_pci->slot,
                            (uint32_t)tmp_pci->function);
                }
                break;
            default:
                break;
            }
            if (devices_pos > old_devices_pos)
            {
                devices = (uint32_t *)kexpand(devices, sizeof(uint32_t));
                devices[devices_pos] = 0x00000000;
            }
        }
        tmp_pci = tmp_pci->next;
    }
    if (!devices_pos)
    {
        kfree(devices);
        return (uint32_t *)NULL;
    }
    else
    {
        return devices;
    }
}

void pci_install()
{
    pci = (pci_t *)kmalloc(sizeof(pci_t));
    memset(pci, 0, sizeof(pci_t));
    pci_t *tmp_pci = pci;
    for (uint8_t bus = 0; bus < 255; bus++)
    {
        for (uint8_t slot = 0; slot < 32; slot++)
        {
            for (uint8_t function = 0; function < 8; function++)
            {
                uint16_t vendor = pci_read(bus, slot, function, 0x00);
                uint16_t device = pci_read(bus, slot, function, 0x02);
                if (vendor != 0xffff)
                {
                    tmp_pci->vendor = vendor;
                    tmp_pci->device = device;
                    tmp_pci->class = (pci_read(bus, slot, function, 0x0a) >> 8) & 0xff;
                    tmp_pci->subclass = (pci_read(bus, slot, function, 0x0a)) & 0xff;
                    tmp_pci->interface = (pci_read(bus, slot, function, 0x08) >> 8) & 0xff;
                    tmp_pci->bus = bus;
                    tmp_pci->slot = slot;
                    tmp_pci->function = function;
                    tmp_pci->next = (pci_t *)kmalloc(sizeof(pci_t));
                    memset(tmp_pci->next, 0, sizeof(pci_t));
                    tmp_pci->next->prev = tmp_pci;
                    tmp_pci = tmp_pci->next;
                }
            }
        }
    }
}