#include <uhci.h>

#define MAX_FRAME_LIST 1024

uhci_t *uhci_dev;

bool uhci_check_irq(uint8_t irq)
{
    uhci_t *tmp_dev = uhci_dev;
    if (!tmp_dev)
        return false;
    while (tmp_dev)
    {
        if (tmp_dev->irq == irq)
            return true;
        tmp_dev = tmp_dev->next;
    }
    return false;
}

void uhci_handler(register_t *regs)
{
    uint16_t status = 0, val = 0;
    uhci_t *tmp_dev = uhci_dev;
    while (tmp_dev)
    {
        if (tmp_dev->irq == regs->int_no - 32)
        {
            val = inw(tmp_dev->status);
            break;
        }
        tmp_dev = tmp_dev->next;
    }
    if (!tmp_dev || !val)
        return;
    if (val & UHCI_STATUS_USBINT)
    {
        status |= UHCI_STATUS_USBINT;
    }
    if (val & UHCI_STATUS_RESUME_DETECT)
    {
        status |= UHCI_STATUS_RESUME_DETECT;
    }
    if (val & UHCI_STATUS_HCHALTED)
    {
        status |= UHCI_STATUS_HCHALTED;
    }
    if (val & UHCI_STATUS_HC_PROCESS_ERROR)
    {
        status |= UHCI_STATUS_HC_PROCESS_ERROR;
    }
    if (val & UHCI_STATUS_USB_ERROR)
    {
        status |= UHCI_STATUS_USB_ERROR;
    }
    if (val & UHCI_STATUS_HOST_SYSTEM_ERROR)
    {
        status |= UHCI_STATUS_HOST_SYSTEM_ERROR;
    }
    outw(tmp_dev->status, status);
    kprintf("[UHCI]: IRQ %u\n", regs->int_no - 32);
}

bool uhci_check_port(uint32_t port)
{
    if (~inw(port) & 0x80)
        return false;
    outw(port, inw(port) & ~0x80);
    if (~inw(port) & 0x80)
        return false;
    outw(port, inw(port) | 0x80);
    if (~inw(port) & 0x80)
        return false;
    outw(port, inw(port) | 0x0A);
    if (inw(port) & 0x0A)
        return false;
    return true;
}

bool uhci_reset_port(uint32_t port)
{
    uint8_t i = 10;
    outw(port, inw(port) | (1 << 9));
    pit_sleep(100);
    outw(port, inw(port) & ~(1 << 9));
    while (i--)
    {
        uint16_t val = inw(port);
        if (~val & (1 << 0))
            return false;
        if (val & ((1 << 3) | (1 << 1)))
        {
            outw(port, val & UHCI_PORT_WRITE_MASK);
            continue;
        }
        if (val & (1 << 2))
            return true;
        outw(port, val | (1 << 2));
    }
    return false;
}

void uhci_add(uint8_t bus, uint8_t slot, uint8_t function)
{
    if (!bus && !slot && !function)
        return;
    uint32_t base = pci_read_bar_address(bus, slot, function, 0x20) & 0xffe0;
    uint32_t version = pci_read_bar_address(bus, slot, function, 0x60);
    uhci_t *new_uhci_dev = (uhci_t *)kmalloc(sizeof(uhci_t));
    memset(new_uhci_dev, 0, sizeof(uhci_t));
    new_uhci_dev->frame_list = (uint32_t *)kmalloc_ap(sizeof(uint32_t) * 1024,
                                                      0x1000,
                                                      &new_uhci_dev->frame_list_physical);
    memset(new_uhci_dev->frame_list, 0, sizeof(uint32_t) * 1024);
    new_uhci_dev->stack_list = (uhci_queue_t *)kmalloc_ap(sizeof(uhci_queue_t) * UHCI_QUEUE_LEN,
                                                          0x10,
                                                          &new_uhci_dev->stack_list_physical);
    memset(new_uhci_dev->stack_list, 0, sizeof(uhci_queue_t) * UHCI_QUEUE_LEN);
    for (uint8_t i = 0; i < UHCI_QUEUE_LEN; i++)
    {
        new_uhci_dev->stack_list[i].horizontal = UHCI_QUEUE_STRUCT_T;
        new_uhci_dev->stack_list[i].vertical = UHCI_QUEUE_STRUCT_T;
    }
    for (uint8_t i = 0; i < UHCI_QUEUE_LEN - 1; i++)
    {
        new_uhci_dev->stack_list[i].horizontal =
            (new_uhci_dev->stack_list_physical + (i + 1) * sizeof(uhci_queue_t)) | UHCI_QUEUE_STRUCT_Q;
    }
    for (uint16_t i = 0; i < 1024; i++)
    {
        uint8_t queue_start = UHCI_QUEUE_Q1;
        for (uint16_t j = 2; j < 256; j *= 2)
        {
            if (((i + 1) % j) == 0)
                queue_start--;
        }
        new_uhci_dev->frame_list[i] =
            (new_uhci_dev->stack_list_physical + queue_start * sizeof(uhci_queue_t)) | UHCI_QUEUE_STRUCT_Q;
    }
    new_uhci_dev->irq = (uint8_t)pci_read_bar_address(bus, slot, function, 0x3C);
    new_uhci_dev->command = base + 0x00;
    new_uhci_dev->status = base + 0x02;
    new_uhci_dev->interrupt_enable = base + 0x04;
    new_uhci_dev->frame_number = base + 0x06;
    new_uhci_dev->frame_list_base_address = base + 0x08;
    new_uhci_dev->start_frame_modify = base + 0x0c;
    new_uhci_dev->port = base + 0x10;
    if (uhci_check_irq(new_uhci_dev->irq))
        goto error;
    pci_write(bus, slot, function, 0x04, 0x05);
    for (uint8_t i = 0; i < 5; i++)
    {
        outw(new_uhci_dev->command, 0x04);
        pit_sleep(100);
        outw(new_uhci_dev->command, 0x00);
    }
    if (inw(new_uhci_dev->command) != 0x00)
        goto error;
    if (inw(new_uhci_dev->status) != 0x20)
        goto error;
    outw(new_uhci_dev->status, 0xff);
    uint8_t sof_backup = inb(new_uhci_dev->start_frame_modify);
    outw(new_uhci_dev->command, 0x02);
    pit_sleep(100);
    if (inw(new_uhci_dev->command) & 0x02)
        goto error;
    outb(new_uhci_dev->start_frame_modify, sof_backup);
    pci_write(bus, slot, function, 0xC0, 0xAF00);
    outw(new_uhci_dev->interrupt_enable, 0x0f);
    outw(new_uhci_dev->frame_number, 0x00);
    outl(new_uhci_dev->frame_list_base_address, new_uhci_dev->frame_list_physical);
    outw(new_uhci_dev->status, 0xffff);
    outw(new_uhci_dev->command, (1 << 7) | (1 << 6) | (1 << 0));
    for (uint32_t port = new_uhci_dev->port; uhci_check_port(port); port += 2)
    {
        if (uhci_reset_port(port))
        {
            if (inw(port) & 1)
            {
                kprintf("\tNew Device: %u\n", port);
            }
        }
    }
    if (uhci_dev)
    {
        uhci_t *tmp = uhci_dev;
        while (tmp->next)
            tmp = tmp->next;
        new_uhci_dev->prev = tmp;
        tmp->next = new_uhci_dev;
    }
    else
    {
        uhci_dev = new_uhci_dev;
    }
    irq_add_handler(new_uhci_dev->irq, uhci_handler);
    return;
error:
    kprintf("Error\n");
    kfree(new_uhci_dev->frame_list);
    kfree(new_uhci_dev->stack_list);
    kfree(new_uhci_dev);
    return;
}

void uhci_service()
{
    uhci_t *tmp_dev = uhci_dev;
    while (true)
    {
        if (!tmp_dev)
        {
            tmp_dev = uhci_dev;
            continue;
        }
        for (uint32_t port = tmp_dev->port; uhci_check_port(port); port += 2)
        {
            uint16_t port = inw(port);
            if (port & (1 << 1))
            {
                outw(port, 1 << 1);
                if (port & (1 << 0))
                {
                    uhci_reset_port(port);
                    kprintf("UHCI: Connected Device");
                }
                else
                {
                    kprintf("UHCI: Disconnected Device");
                }
            }
        }
        tmp_dev = tmp_dev->next;
    }
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
    task_add(task_create("UHCI", uhci_service, (void *)NULL));
}
