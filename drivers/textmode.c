#include <textmode.h>

uint16_t cursor_pos = -1;

uint16_t get_cursor_pos()
{
    if (cursor_pos == -1)
    {
        cursor_pos = 0;
        outb(VIDEO_CONTROL_PIN, 14);
        cursor_pos = inb(VIDEO_DATA_PIN) << 8;
        outb(VIDEO_CONTROL_PIN, 15);
        cursor_pos += inb(VIDEO_DATA_PIN);
    }
    return cursor_pos;
}

void set_cursor_pos(uint8_t x, uint8_t y)
{
    uint16_t pos = y * 80 + x;
    cursor_pos = pos;
    outb(0x3D4, 14);
    outb(0x3D5, pos >> 8);
    outb(0x3D4, 15);
    outb(0x3D5, pos);
}

void clear_screen(uint8_t color)
{
    if (!color)
        color = 0x0f;
    uint16_t *screen = VIDEO_MEMORY;
    for (uint16_t i = 0; i < 2000; i++)
        *screen++ = (((color & 0x0f) << 8) | (0x20 & 0xff));
    set_cursor_pos(0, 0);
}

void scroll()
{
    uint8_t *screen = VIDEO_MEMORY;
    for (uint16_t i = 0; i < 4000; i++)
        screen[i] = screen[i + 160];
}

void move_next_cursor()
{
    uint16_t curpos = get_cursor_pos();
    uint8_t y = curpos / 80;
    uint8_t x = curpos % 80;
    x += 1;
    if (x == 80)
    {
        y += 1;
        x = 0;
    }
    if (y == 25)
    {
        scroll();
        x = 0;
        y -= 1;
    }
    set_cursor_pos(x, y);
}

void dos_print_char(char c, char colors, char toblink)
{
    uint32_t curpos = get_cursor_pos() * 2;
    uint8_t *screen = (char *)curpos + VIDEO_MEMORY;
    if (c == '\n')
    {
        next_line();
        curpos = get_cursor_pos() * 2;
        screen = (char *)curpos + VIDEO_MEMORY;
        return;
    }
    *screen = c;
    *(screen + 1) = c == toblink ? colors | 0x80 : colors;
    screen += 2;
    move_next_cursor();
}

void dos_print(char *text, char colors, char toblink)
{
    if (!text)
        return;
    uint32_t curpos = get_cursor_pos() * 2;
    char *screen = (char *)curpos + VIDEO_MEMORY;
    for (uint64_t i = 0; text[i] != '\0'; i++)
    {
        dos_print_char(text[i], colors, toblink);
    }
}

void dos_print_hex(uint32_t hexcode, char colors, char toblink)
{
    int32_t tmp;

    dos_print("0x", colors, toblink);

    char noZeroes = 1;

    for (int32_t i = 28; i > 0; i -= 4)
    {
        tmp = (hexcode >> i) & 0xF;
        if (tmp == 0 && noZeroes != 0)
        {
            continue;
        }

        if (tmp >= 0xA)
        {
            noZeroes = 0;
            dos_print_char(tmp - 0xA + 'A', colors, toblink);
        }
        else
        {
            noZeroes = 0;
            dos_print_char(tmp + '0', colors, toblink);
        }
    }

    tmp = hexcode & 0xF;
    if (tmp >= 0xA)
    {
        dos_print_char(tmp - 0xA + 'A', colors, toblink);
    }
    else
    {
        dos_print_char(tmp + '0', colors, toblink);
    }
}

void next_line()
{
    uint16_t curpos = get_cursor_pos();
    uint8_t y = curpos / 80;
    uint8_t x = curpos % 80;
    x = 0;
    y += 1;
    if (y == 25)
    {
        scroll();
        x = 0;
        y -= 1;
    }
    set_cursor_pos(x, y);
}