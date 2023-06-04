#include <mouse.h>

#define PS2Leftbutton 0x01
#define PS2Middlebutton 0x04
#define PS2Rightbutton 0x02
#define PS2XSign 0x10
#define PS2YSign 0x20
#define PS2XOverflow 0x40
#define PS2YOverflow 0x80

void mouse_wait(bool wait_type)
{
    uint32_t timeout = 1000000;
    if (!wait_type)
    {
        while (timeout--)
        {
            if (inb(0x64) & 0x01)
                return;
        }
    }
    else
    {
        while (timeout--)
        {
            if (!(inb(0x64) & 0x02))
                return;
        }
    }
}

uint8_t mouse_read()
{
    mouse_wait(0);
    return inb(0x60);
}

void mouse_write(uint8_t value)
{
    mouse_wait(1);
    outb(0x64, 0xD4);
    mouse_wait(1);
    outb(0x60, value);
}

void mouse_set_sample_rate(uint8_t sample_rate)
{
    uint8_t status = 0;
    outb(0x64, 0xD4);
    outb(0x60, 0xF3);
    while (inb(0x64) & 0x01)
        ;
    status = inb(0x60);
    outb(0x64, 0xD4);
    outb(0x60, sample_rate);
    while (inb(0x64) & 0x01)
        ;
    status = inb(0x60);
}

uint8_t mouse_get_id()
{
    mouse_write(0xF5);
    if (mouse_read() == 0xFA)
    {
        mouse_write(0xF2);
        uint8_t data = mouse_read();
        if (data == 0xFA)
            return data;
    }
    return NULL;
}

mouse_t mouse_ps2;
Point_t mouse_temp;
int8_t packet_cache[6], packet_index = 0;
static bool xOverflow = false, yOverflow = false, xNegative = false, yNegative = false;

int16_t mouse_get_packet(mouse_t *mouse, int8_t packet)
{
    if (!mouse)
        return 0;
    if (packet < 0 || packet > 4)
        return 0;
    int16_t backup = 0;
    if (packet > 2)
    {
        if (packet == 3)
            backup = mouse->delta_x;
        else if (packet == 4)
            backup = mouse->delta_y;
    }
    else
    {
        backup = mouse->packet[packet];
    }
    if (packet < 3)
    {
        mouse->packet[packet] = mouse->delta_packet[packet];
    }
    else
    {
        if (packet == 3)
            mouse->delta_x = 0;
        else if (packet == 4)
            mouse->delta_y = 0;
    }
    return backup;
}

void mouse_handle_packet(mouse_t *mouse, uint8_t data)
{
    if (!mouse)
        return;
    switch (packet_index)
    {
    case 0:
        if (data & 0x08 == 0)
            break;
        if (data & PS2Leftbutton)
        {
            mouse->delta_packet[0] = true;
            mouse->packet[0] = true;
        }
        else
        {
            mouse->delta_packet[0] = false;
        }

        if (data & PS2Middlebutton)
        {
            mouse->delta_packet[1] = true;
            mouse->packet[1] = true;
        }
        else
        {
            mouse->delta_packet[1] = false;
        }

        if (data & PS2Rightbutton)
        {
            mouse->delta_packet[2] = true;
            mouse->packet[2] = true;
        }
        else
        {
            mouse->delta_packet[2] = false;
        }

        if (data & PS2XOverflow)
        {
            xOverflow = true;
        }

        else
        {
            xOverflow = false;
        }

        if (data & PS2YOverflow)
        {
            yOverflow = true;
        }
        else
        {
            yOverflow = false;
        }

        if (data & PS2XSign)
        {
            xNegative = true;
        }
        else
        {
            xNegative = false;
        }

        if (data & PS2YSign)
        {
            yNegative = true;
        }
        else
        {
            yNegative = false;
        }

        packet_cache[0] = data;
        break;
    case 1:
        packet_cache[1] = data;
        if (xOverflow)
            break;
        mouse->delta_x += packet_cache[1];
        break;
    case 2:
        packet_cache[2] = data;
        if (yOverflow)
            break;
        mouse->delta_y += packet_cache[2];
        break;
    }
    packet_index = (packet_index + 1) % mouse_ps2.id;
}

void mouse_handler(register_t *r)
{
    if (~inb(0x64) & 0x01)
        return;
    uint8_t mousedata = inb(0x60);
    mouse_handle_packet(&mouse_ps2, mousedata);
}

void mouse_install()
{
    uint8_t status;
    mouse_wait(1);
    outb(0x64, 0xA8);
    mouse_wait(1);
    outb(0x64, 0x20);
    mouse_wait(0);
    status = inb(0x60) | 2;
    mouse_wait(1);
    outb(0x64, 0x60);
    mouse_wait(1);
    outb(0x60, status);
    mouse_write(0xF6);
    mouse_read();
    mouse_write(0xF4);
    mouse_read();
    memset(&mouse_ps2, 0, sizeof(mouse_t));
    mouse_ps2.id = 3;
    irq_add_handler(12, mouse_handler);
}