#include <window.h>

#define IGNORE(W, H) 0, 0, (uint16_t)W, (uint16_t)H

window_t *start_window, *current_window;
uint32_t window_id = 0;
uint32_t window_z = 0;
uint32_t window_current_z = 0;
uint32_t *WindowGUI;

bool ColisionDetector(float x1, float y1, float width1, float height1, float x2, float y2, float width2, float height2)
{
    if (x1 < x2 + width2 && x1 + width1 > x2 && y1 < y2 + height2 && y1 + height1 > y2)
        return true;
    else
        return false;
}

void window_setup()
{
    WindowGUI = tga_parse("system/app/ilframework/assets/WindowGui_Classic.tga");
}

window_t *window_create(uint16_t width, uint16_t height, uint16_t x, uint16_t y)
{
    window_t *ret = kmalloc(sizeof(window_t));
    memset(ret, 0, sizeof(window_t));
    ret->id = window_id++;
    ret->x = x;
    ret->y = y;
    ret->z = window_z++;
    ret->width = width;
    ret->height = height;
    ret->off_frame = canvas_create(NULL, width, height, 32);
    ret->on_frame = canvas_create(NULL, width, height, 32);
    return ret;
}

window_t *window_get_ordered_z()
{
    if (!start_window)
        return NULL;
    window_t *tmp_window = start_window;
    while (tmp_window)
    {
        if (tmp_window->z == window_current_z)
            return tmp_window;
        tmp_window = tmp_window->next;
    }
    return NULL;
}

bool window_colision_check(window_t *window, float x, float y, float width, float height)
{
    if (!start_window || !window)
        return false;
    window_t *tmp_window = start_window;
    bool tmp_bool = false;
    while (tmp_window)
    {
        int16_t diffX = 0, diffY = 0, offX = 0, offY = 0;
        diffX = window->x - tmp_window->x;
        diffY = window->y - tmp_window->y;
        diffX = diffX > window->width ? window->width : diffX;
        diffX = diffX > window->height ? window->height : diffY;
        tmp_bool = ColisionDetector(
            window->x,
            window->y,
            window->width + 5,
            window->height + 16,
            x,
            y,
            width,
            height);
        tmp_window = tmp_window->next;
    }
    return tmp_bool;
}

void window_set_focus(window_t *window)
{
    window_t *tmp_window = start_window;
    while (tmp_window)
    {
        if (tmp_window->id == window->id)
        {
            if (tmp_window->focus)
                break;
            uint32_t old_z = tmp_window->z;
            window_t *old_window = start_window, *tmp2_window = start_window;
            while (tmp2_window)
            {
                if (tmp2_window->z > old_z)
                    old_z = tmp2_window->z, old_window = tmp2_window;
                tmp2_window = tmp2_window->next;
            }
            if (tmp_window->z == old_z)
                break;
            old_window->z = tmp_window->z;
            tmp_window->z = old_z;
            tmp_window->focus = true;
            break;
        }
        else
        {
            tmp_window->focus = false;
        }
        tmp_window = tmp_window->next;
    }
}

bool window_draw(canvas_t *canvas, window_t *window)
{
    if (!start_window || !window || !canvas)
        return false;
    if (window->z != window_current_z)
        return true;
    canvas_drawimage(canvas, WindowGUI,
                     window->x, window->y + window->height + 16, window->width + 10, 5,
                     36, 17, 4, 5);
    canvas_drawimage(canvas, WindowGUI,
                     window->x, window->y, 3, 16,
                     0, 0, 3, 16);
    canvas_drawimage(canvas, WindowGUI,
                     window->x + 3, window->y, window->width + 4, 16,
                     3, 0, 29, 16);
    canvas_drawimage(canvas, WindowGUI,
                     window->x + window->width + 7, window->y, 3, 16,
                     33, 0, 3, 16);
    canvas_drawimage(canvas, WindowGUI,
                     window->x, window->y + 16, 5, window->height + 2,
                     36, 0, 5, 7);
    canvas_drawimage(canvas, WindowGUI,
                     window->x + window->width + 5, window->y + 16, 5, window->height + 2,
                     36, 9, 5, 7);
    canvas_copy(canvas, window->on_frame, window->x + 5, window->y + 16);
    return false;
}

bool window_node_add(window_t *window)
{
    if (!start_window)
    {
        start_window = current_window = window;
        return false;
    }
    if (!start_window || !window)
        return true;
    window_t *tmp_window = start_window;
    while (tmp_window->next)
        tmp_window = tmp_window->next;
    tmp_window->next = window;
    tmp_window->next->prev = tmp_window;
    return false;
}

bool window_node_remove(uint32_t id)
{
    if (!id)
    {
        start_window = NULL;
        return true;
    }
    if (!start_window)
        return true;
    window_t *tmp_window = start_window;
    while (tmp_window->next->id != id && tmp_window->next)
        tmp_window = tmp_window->next;
    if (!tmp_window->next || !tmp_window->next->id || tmp_window->next->id != id)
        return true;
    if (!tmp_window->next->prev)
        return true;
    tmp_window->next->prev->next = tmp_window->next->next;
    return false;
}