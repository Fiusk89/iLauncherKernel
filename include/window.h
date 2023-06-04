#ifndef WINDOW_H
#define WINDOW_H
#include <kernel.h>

typedef struct window
{
    uint32_t id, flags, z;
    int16_t x, y, old_x, old_y;
    uint16_t width, height;
    struct canvas *off_frame, *on_frame;
    bool draw_frame, is_frame, focus;
    struct window *prev;
    struct window *next;
} __attribute__((packed)) window_t;

typedef struct window_event
{
    float MovementX, MovementY;
    float PositionX, PositionY;
    bool MouseL, MouseM, MouseR;
} __attribute__((packed)) window_event_t;

extern window_t *start_window, *current_window;
extern uint32_t window_id;
extern uint32_t window_z;
extern uint32_t window_current_z;

void window_setup();
bool window_draw(struct canvas *canvas, window_t *window);
bool window_colision_check(window_t *window, float x, float y, float width, float height);
void window_set_focus(window_t *window);
window_t *window_get_ordered_z();
window_t *window_create(uint16_t width, uint16_t height, uint16_t x, uint16_t y);
bool window_node_add(window_t *window);
bool window_node_remove(uint32_t id);
#endif