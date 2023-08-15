#pragma once
#ifndef SCREEN_H
#define SCREEN_H
#include <kernel.h>

static uint32_t vga_colors[16] = {
    0x000000,
    0x0000AA,
    0x00AA00,
    0x00AAAA,
    0xAA0000,
    0xAA00AA,
    0xAA5500,
    0xAAAAAA,
    0x555555,
    0x5555FF,
    0x55FF55,
    0x55FFFF,
    0xFF5555,
    0xFF55FF,
    0xFFFF55,
    0xFFFFFF,
};

typedef struct screen_mode_info
{
    /*
        bit 1: linear_framebuffer
        bit 2: is_vga
    */
    uint16_t flags;
    uint32_t framebuffer;
    uint16_t mode;
    uint16_t width, twidth;
    uint16_t height, theight;
    uint16_t pitch, tpitch;
    uint8_t bpp;
    struct screen_mode_info *prev;
    struct screen_mode_info *next;
} screen_mode_info_t;

typedef struct screen_info
{
    uint8_t mode;
    uint8_t cursor;
    uint8_t cursor_start, cursor_end;
    uint32_t cursor_position;
    uint16_t video_modes_length;
    screen_mode_info_t *video_modes;
    screen_mode_info_t *current_video_mode;
    uint8_t (*configure)();
    uint8_t (*unconfigure)();
    uint16_t *text_framebuffer;
    void *graphic_framebuffer;
    struct screen_info *prev;
    struct screen_info *next;
} screen_info_t;

void screen_install();
screen_info_t *screen_get_info();
void screen_add(screen_info_t *screen);
void screen_remove(screen_info_t *screen);
#endif