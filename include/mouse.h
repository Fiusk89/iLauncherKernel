#ifndef MOUSE_H
#define MOUSE_H
#include <kernel.h>

typedef struct mouse
{
    uint8_t id;
    int8_t packet[3];
    int8_t delta_packet[3];
    int16_t delta_x, delta_y;
} mouse_t;

extern mouse_t mouse_ps2;

int16_t mouse_get_packet(mouse_t *, int8_t);
void mouse_install();
#endif