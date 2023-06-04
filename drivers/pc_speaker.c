#include <pc_speaker.h>

uint8_t pc_speaker_value = 0, pc_speaker_sync = 10;

void pc_speaker_enable()
{
    outb(0x61, inb(0x61) | 0x03);
}

void pc_speaker_disable()
{
    outb(0x61, inb(0x61) & 0xFC);
}

void pc_speaker_service()
{
loop:
    uint8_t value = ((pc_speaker_value * 54) / 255);
    if (!value)
        pc_speaker_disable();
    else
        pc_speaker_enable();
    outb(0x42, value);
    sleep(5);
    outb(0x42, value);
    sleep(5);
    goto loop;
}

void pc_speaker_set_value(uint8_t value)
{
    pc_speaker_value = value;
    sleep(pc_speaker_sync);
}

void pc_speaker_playsound(uint8_t *data, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++)
        pc_speaker_set_value(*data++);
    pc_speaker_set_value(0);
}

void pc_speaker_install()
{
    outb(0x43, 0xb6);
    outb(0x42, 0);
    outb(0x42, 0);
    pc_speaker_enable();
    sleep(PIT_HZ * 12);
    pc_speaker_disable();
    task_add(task_create("pc speaker service", pc_speaker_service, 0));
}