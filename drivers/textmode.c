#include <textmode.h>

uint16_t cursor_pos = 0;
uint32_t VIDEO_MEMORY = KERNEL_P2V(0xb8000);
uint16_t vga_width = 80, vga_height = 25;
uint8_t vga_cursor_color = 0x07;
extern uint32_t screen_cursor_position;

uint32_t get_cursor_pos()
{
    if (cursor_pos == -1)
    {
        cursor_pos = 0;
        outb(VIDEO_CONTROL_PIN, 14);
        cursor_pos = inb(VIDEO_DATA_PIN) << 8;
        outb(VIDEO_CONTROL_PIN, 15);
        cursor_pos += inb(VIDEO_DATA_PIN);
    }
    return screen_cursor_position;
}

void set_cursor_pos(uint16_t x, uint16_t y)
{
    uint16_t pos = y * 80 + x;
    cursor_pos = pos;
    outb(0x3D4, 14);
    outb(0x3D5, pos >> 8);
    outb(0x3D4, 15);
    outb(0x3D5, pos);
    screen_cursor_position = y * vga_width + x;
}

void clear_screen()
{
    uint16_t *screen = (uint16_t *)VIDEO_MEMORY;
    for (uint32_t i = 0; i < vga_width * vga_height; i++)
        *screen++ = (((vga_cursor_color & 0x0f) << 8) | (' ' & 0xff));
    set_cursor_pos(0, 0);
}

void scroll()
{
    uint16_t *screen = (uint16_t *)VIDEO_MEMORY;
    for (uint32_t i = 0; i < vga_width * (vga_height - 1); i++)
        screen[i] = screen[i + vga_width];
    for (uint32_t i = vga_width * (vga_height - 1); i < vga_width * vga_height; i++)
        screen[i] = (((vga_cursor_color & 0x0f) << 8) | (' ' & 0xff));
}

void remove_last_char()
{
    uint16_t y = screen_cursor_position / vga_width;
    uint16_t x = screen_cursor_position % vga_width;
    if (x == 0 && y > 0)
    {
        x = vga_width - 1, y--;
    }
    else if (x > 0)
    {
        x--;
    }
    set_cursor_pos(x, y);
    uint16_t *screen = (uint16_t *)(VIDEO_MEMORY + (screen_cursor_position * sizeof(uint16_t)));
    *screen = (((vga_cursor_color & 0x0f) << 8) | (' ' & 0xff));
}

void move_next_cursor()
{
    uint16_t y = screen_cursor_position / vga_width;
    uint16_t x = screen_cursor_position % vga_width;
    x += 1;
    if (x == vga_width)
    {
        y += 1;
        x = 0;
    }
    if (y == vga_height)
    {
        scroll();
        x = 0;
        y -= 1;
    }
    set_cursor_pos(x, y);
}

void dos_print_char(char c, char colors, char toblink)
{
    uint32_t curpos = screen_cursor_position * sizeof(uint16_t);
    uint8_t *screen = (uint8_t *)curpos + VIDEO_MEMORY;
    switch (c)
    {
    case '\n':
        next_line();
        curpos = screen_cursor_position * sizeof(uint16_t);
        screen = (uint8_t *)curpos + VIDEO_MEMORY;
        return;
    }
    *screen = c;
    *(screen + 1) = c == toblink ? colors | 0x80 : colors;
    move_next_cursor();
}

void dos_print(char *text, char colors, char toblink)
{
    if (!text)
        return;
    for (uint32_t i = 0; text[i] != '\0'; i++)
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
    uint16_t y = screen_cursor_position / vga_width;
    uint16_t x = screen_cursor_position % vga_width;
    x = 0;
    y += 1;
    if (y == vga_height)
    {
        scroll();
        x = 0;
        y -= 1;
    }
    set_cursor_pos(x, y);
}