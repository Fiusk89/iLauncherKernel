#include <pit.h>

pit_t pit_divisor = 0, pit_delay = 0;
bool pit_beep = false;

void pit_handler(register_t *r)
{
    if (current_task)
    {
        current_task->active_time++;
        schedule();
    }
    else
    {
        if (pit_delay)
            pit_delay--;
    }
}

void pit_timer_hz(uint32_t hz)
{
    uint32_t divisor = 1193180 / hz;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xff);
    outb(0x40, (divisor >> 8) & 0xff);
}

void pit_prepare_sleep(uint32_t microseconds)
{
    outb(0x61, inb(0x61) & ~2);
    outb(0x43, 0xB0);
    pit_divisor = PIT_FREQUENCY / (1000000 / microseconds);
}

void pit_install()
{
    pit_timer_hz(PIT_HZ);
    irq_add_handler(0, pit_handler);
}