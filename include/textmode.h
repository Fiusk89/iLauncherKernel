#ifndef VIDEO_H
#define VIDEO_H
#include <kernel.h>

#define CHAR_PER_LINE80
#define VIDEO_MEMORY KERNEL_P2V(0xb8000)
#define VIDEO_CONTROL_PIN 0x3D4
#define VIDEO_DATA_PIN 0x3D5

uint16_t get_cursor_pos();
void set_cursor_pos(uint8_t x, uint8_t y);
void dos_print_char(char c, char colors, char toblink);
void dos_print(char *text, char colors, char toblink);
void dos_print_hex(uint32_t hexcode, char colors, char toblink);
void rm_char_in_pos();
void clear_screen(uint8_t color);
void next_line();
void scroll();
#endif