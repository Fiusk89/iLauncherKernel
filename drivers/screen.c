#include <screen.h>

screen_info_t *start_screen;
screen_info_t *current_screen;
uint8_t screen_is_empty = true;
uint8_t screen_cursor = false;
uint8_t screen_cursor_start = 14, screen_cursor_end = 16;
uint32_t screen_cursor_position = 0;

uint8_t screen_text_mask[8] = {
    128,
    64,
    32,
    16,
    8,
    4,
    2,
    1,
    0,
};

void screen_service();

void screen_install()
{
    task_add(task_create("Screen Manager", screen_service, NULL));
}

screen_info_t *screen_get_info()
{
    return current_screen;
}

void screen_add(screen_info_t *screen)
{
    if (!screen)
        return;
    if (!start_screen)
    {
        start_screen = current_screen = screen;
        screen_is_empty = false;
        return;
    }
    screen_info_t *tmp_screen = start_screen;
    while (tmp_screen->next)
        tmp_screen = tmp_screen->next;
    screen->next = NULL;
    screen->prev = tmp_screen;
    tmp_screen->next = screen;
}

void screen_drawchar(void *framebuffer, uint32_t pitch, uint8_t chr, uint8_t color, uint8_t bpp)
{
    if (!framebuffer)
        return;
    uint8_t *gylph = font + 2 + (chr * font[1]);
    for (uint8_t y = 0; y < font[1]; y++)
    {
        uint32_t *framebuffer_offset32 = (uint32_t *)framebuffer;
        uint24_t *framebuffer_offset24 = (uint24_t *)framebuffer;
        uint16_t *framebuffer_offset16 = (uint16_t *)framebuffer;
        for (uint8_t x = 0; x < font[0]; x++)
        {
            if (gylph[y] & screen_text_mask[x])
            {
                uint8_t tmp = color & 0x0f;
                switch (bpp)
                {
                case 32:
                    (*framebuffer_offset32++) = vga_colors[tmp];
                    break;
                case 24:
                    (*framebuffer_offset24++).value = vga_colors[tmp];
                    break;
                case 16:
                    (*framebuffer_offset16++) = vga_colors[tmp];
                    break;
                default:
                    break;
                }
            }
            else
            {
                uint8_t tmp = (color >> 4) & 0x0f;
                switch (bpp)
                {
                case 32:
                    (*framebuffer_offset32++) = vga_colors[tmp];
                    break;
                case 24:
                    (*framebuffer_offset24++).value = vga_colors[tmp];
                    break;
                case 16:
                    (*framebuffer_offset16++) = vga_colors[tmp];
                    break;
                default:
                    break;
                }
            }
        }
        framebuffer += pitch;
    }
}

void screen_service()
{
    pit_t saved_timer;
    while (true)
    {
        if (screen_is_empty || !start_screen || !current_screen)
            continue;
        if (current_screen->current_video_mode->flags & (1 << 1))
        {
            uint8_t bpp[2] = {
                current_screen->current_video_mode->bpp,
                round((float)current_screen->current_video_mode->bpp / 8.0f),
            };
            uint32_t framebuffer_size = current_screen->current_video_mode->pitch *
                                        current_screen->current_video_mode->height;
            uint16_t width = current_screen->current_video_mode->pitch;
            uint16_t height = current_screen->current_video_mode->height;
            uint16_t cursor_pos[2] = {
                (screen_cursor_position % current_screen->current_video_mode->twidth) * font[0],
                (screen_cursor_position / current_screen->current_video_mode->twidth) * font[1],
            };
            uint8_t char_size[2] = {
                font[0] * bpp[1],
                font[1] * bpp[1],
            };
            uint8_t *textbuffer_offset = (uint8_t *)current_screen->text_framebuffer;
            uint8_t *textbuffer_offset2 = (uint8_t *)&current_screen->text_framebuffer[screen_cursor_position];
            for (uint32_t ty = 0; ty < framebuffer_size; ty += current_screen->current_video_mode->pitch * font[1])
            {
                void *framebuffer_offset = (void *)(current_screen->current_video_mode->framebuffer + ty);
                void *framebuffer_offset_old = framebuffer_offset;
                for (uint32_t tx = 0; tx < width; tx += char_size[0])
                {
                    screen_drawchar(framebuffer_offset, width, textbuffer_offset[0], textbuffer_offset[1], bpp[0]);
                    if (current_task->active_time - saved_timer > 120)
                        saved_timer = current_task->active_time, screen_cursor = !screen_cursor;
                    for (uint8_t y = screen_cursor_start; y < screen_cursor_end; y++)
                    {
                        if (screen_cursor_position >
                            current_screen->current_video_mode->twidth *
                                current_screen->current_video_mode->theight)
                            break;
                        void *framebuffer_offset2 = (void *)(current_screen->current_video_mode->framebuffer +
                                                             (cursor_pos[0] * bpp[1]) +
                                                             (current_screen->current_video_mode->pitch * (y + cursor_pos[1])));
                        uint16_t *framebuffer_offset16 = (uint16_t *)framebuffer_offset2;
                        uint24_t *framebuffer_offset24 = (uint24_t *)framebuffer_offset2;
                        uint32_t *framebuffer_offset32 = (uint32_t *)framebuffer_offset2;
                        if (screen_cursor)
                            for (uint8_t x = 0; x < 8; x++)
                            {
                                if (bpp[0] == 32)
                                    (*framebuffer_offset32++) = vga_colors[textbuffer_offset2[1] & 0x0f];
                                else if (bpp[0] == 24)
                                    (*framebuffer_offset24++).value = vga_colors[textbuffer_offset2[1] & 0x0f];
                                else if (bpp[0] == 16)
                                    (*framebuffer_offset16++) = vga_colors[textbuffer_offset2[1] & 0x0f];
                            }
                        else if (!textbuffer_offset2[0])
                            for (uint8_t x = 0; x < 8; x++)
                            {
                                if (bpp[0] == 32)
                                    (*framebuffer_offset32++) = vga_colors[0];
                                else if (bpp[0] == 24)
                                    (*framebuffer_offset24++).value = vga_colors[0];
                                else if (bpp[0] == 16)
                                    (*framebuffer_offset16++) = vga_colors[0];
                            }
                    }
                    framebuffer_offset += char_size[0];
                    textbuffer_offset += 2;
                }
                framebuffer_offset = framebuffer_offset_old;
            }
        }
    }
}