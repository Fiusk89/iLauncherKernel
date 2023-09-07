#include <uhci.h>

#define MAX_FRAME_LIST 1024

uhci_t *uhci_dev;

void uhci_handler(register_t *regs)
{
    kprintf("[UHCI]: IRQ %u\n", regs->int_no - 32);
}

void uhci_init_port(uint32_t port)
{
    uint16_t port_value = ((inw(port) | 0x200) & 0x324e) + 0x80;
    outw(port, port_value);
    pit_delay = 100;
    while (pit_delay)
        ;
    outw(port, port_value & 0xfdff);
    pit_delay = 100;
    while (pit_delay)
        ;
}

void uhci_add(uint8_t bus, uint8_t slot, uint8_t function)
{
    if (!bus && !slot && !function)
        return;
    uhci_t *new_uhci_driver = (uhci_t *)kmalloc(sizeof(uhci_t));
    memset(new_uhci_driver, 0, sizeof(uhci_t));
    uint32_t base = pci_read_bar_address(bus, slot, function, 0x20) & 0xffe0;
    uint32_t version = pci_read_bar_address(bus, slot, function, 0x60);
    new_uhci_driver->irq = (uint8_t)pci_read_bar_address(bus, slot, function, 0x3C);
    new_uhci_driver->command = base + 0x00;
    new_uhci_driver->status = base + 0x02;
    new_uhci_driver->interrupt_enable = base + 0x04;
    new_uhci_driver->frame_number = base + 0x06;
    new_uhci_driver->frame_list_base_address = base + 0x08;
    new_uhci_driver->start_frame_modify = base + 0x0c;
    new_uhci_driver->port1 = base + 0x10;
    new_uhci_driver->port2 = base + 0x12;
    pci_write(bus, slot, function, 0x04, 0x05);
    for (uint8_t i = 0; i < 5; i++)
    {
        outw(new_uhci_driver->command, 0x0004);
        pit_delay = 100;
        while (pit_delay)
            ;
        outw(new_uhci_driver->command, 0x0000);
    }
    if (inw(new_uhci_driver->command) != 0x0000)
        goto error;
    if (inw(new_uhci_driver->status) != 0x0020)
        goto error;
    outw(new_uhci_driver->status, 0xff);
    uint8_t sof_backup = inb(new_uhci_driver->start_frame_modify);
    outw(new_uhci_driver->command, 0x02);
    pit_delay = 100;
    while (pit_delay)
        ;
    if (inw(new_uhci_driver->command) & 0x02)
        goto error;
    outb(new_uhci_driver->start_frame_modify, sof_backup);
    pci_write(bus, slot, function, 0xC0, 0xAF00);
    irq_add_handler(new_uhci_driver->irq, uhci_handler);
    if (uhci_dev)
    {
        uhci_t *tmp = uhci_dev;
        while (tmp->next)
            tmp = tmp->next;
        new_uhci_driver->prev = tmp;
        tmp->next = new_uhci_driver;
    }
    else
    {
        uhci_dev = new_uhci_driver;
    }
    return;
error:
    kprintf("Error\n");
    kfree(new_uhci_driver->frame_list);
    kfree(new_uhci_driver);
    return;
}

void uhci_install()
{
    uint32_t *uhci_devices = pci_find_devices(3, 0x0C, 0x03, 0x00);
    if (!uhci_devices)
        return;
    kprintf("UHCI:\n");
    for (uint8_t i = 0; uhci_devices[i]; i++)
    {
        kprintf("\tbus: %x, slot: %x, function: %x\n",
                uhci_devices[i] & 0xff,
                (uhci_devices[i] >> 8) & 0xff,
                (uhci_devices[i] >> 16) & 0xff);
        uhci_add(uhci_devices[i] & 0xff, (uhci_devices[i] >> 8) & 0xff, (uhci_devices[i] >> 16) & 0xff);
    }
    kfree(uhci_devices);
}