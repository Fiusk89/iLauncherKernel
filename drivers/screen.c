#include <screen.h>

screen_info_t *start_screen;
screen_info_t *current_screen;
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
        return;
    }
    screen_info_t *tmp_screen = start_screen;
    while (tmp_screen->next)
        tmp_screen = tmp_screen->next;
    tmp_screen->next = screen;
    tmp_screen->next->prev = tmp_screen;
}

void screen_service()
{
    pit_t saved_timer = current_task->active_time;
    while (true)
    {
        if (!start_screen || !current_screen)
        {
            sleep(500);
            continue;
        }
        if (current_screen->current_video_mode->flags & (1 << 1))
        {
            uint16_t width = current_screen->current_video_mode->width;
            uint16_t height = current_screen->current_video_mode->height;
            uint8_t bpp[2] = {
                current_screen->current_video_mode->bpp,
                round((float)current_screen->current_video_mode->bpp / 8.0f),
            };
            uint16_t cursor_pos[2] = {
                screen_cursor_position % current_screen->current_video_mode->twidth,
                screen_cursor_position / current_screen->current_video_mode->twidth,
            };
            uint8_t *textbuffer_offset = (uint8_t *)(current_screen->text_framebuffer);
            uint8_t *textbuffer_offset2 = (uint8_t *)(current_screen->text_framebuffer +
                                                      (sizeof(uint16_t) * (screen_cursor_position * 8)));
            for (uint16_t ty = 0; ty < height; ty += 16)
            {
                for (uint16_t tx = 0; tx < width; tx += 8)
                {
                    if (current_task->active_time - saved_timer >= 150)
                        screen_cursor = screen_cursor ? false : true, saved_timer = current_task->active_time;
                    uint8_t *gylph = font + 2 + (textbuffer_offset[0] * font[1]);
                    for (uint8_t y = 0; y < 16; y++)
                    {
                        for (uint8_t x = 0; x < 8; x++)
                        {
                            if (!textbuffer_offset[0])
                                break;
                            void *framebuffer_offset = (void *)(current_screen->current_video_mode->framebuffer +
                                                                ((ty + y) *
                                                                     current_screen->current_video_mode->width +
                                                                 (tx + x)) *
                                                                    bpp[1]);
                            uint16_t *framebuffer_offset16 = (uint16_t *)framebuffer_offset;
                            uint24_t *framebuffer_offset24 = (uint24_t *)framebuffer_offset;
                            uint32_t *framebuffer_offset32 = (uint24_t *)framebuffer_offset;
                            if (gylph[y] & screen_text_mask[x])
                            {
                                if (bpp[0] == 32)
                                    *framebuffer_offset32 = vga_colors[textbuffer_offset2[1] & 0x0f];
                                else if (bpp[0] == 24)
                                    framebuffer_offset24->value = vga_colors[textbuffer_offset2[1] & 0x0f];
                                else if (bpp[0] == 16)
                                    *framebuffer_offset16 = vga_colors[textbuffer_offset2[1] & 0x0f];
                            }
                            else
                            {
                                if (bpp[0] == 32)
                                    *framebuffer_offset32 = vga_colors[(textbuffer_offset2[1] >> 4) & 0x0f];
                                else if (bpp[0] == 24)
                                    framebuffer_offset24->value = vga_colors[(textbuffer_offset2[1] >> 4) & 0x0f];
                                else if (bpp[0] == 16)
                                    *framebuffer_offset16 = vga_colors[(textbuffer_offset2[1] >> 4) & 0x0f];
                            }
                        }
                    }
                    for (uint8_t y = screen_cursor_start; y < screen_cursor_end; y++)
                    {
                        if (cursor_pos[0] * 8 + cursor_pos[1] * 16 >
                            current_screen->current_video_mode->width *
                                current_screen->current_video_mode->height)
                            break;
                        void *framebuffer_offset = (void *)(current_screen->current_video_mode->framebuffer +
                                                            (cursor_pos[0] * 8 * bpp[1]) +
                                                            (current_screen->current_video_mode->pitch * (y + cursor_pos[1] * 16)));
                        uint16_t *framebuffer_offset16 = (uint16_t *)framebuffer_offset;
                        uint24_t *framebuffer_offset24 = (uint24_t *)framebuffer_offset;
                        uint32_t *framebuffer_offset32 = (uint32_t *)framebuffer_offset;
                        if (screen_cursor)
                            for (uint8_t x = 0; x < 8; x++)
                            {
                                if (bpp[0] == 32)
                                    framebuffer_offset32[x] = vga_colors[textbuffer_offset2[1] & 0x0f];
                                else if (bpp[0] == 24)
                                    framebuffer_offset24[x].value = vga_colors[textbuffer_offset2[1] & 0x0f];
                                else if (bpp[0] == 16)
                                    framebuffer_offset16[x] = vga_colors[textbuffer_offset2[1] & 0x0f];
                            }
                        else if (!textbuffer_offset2[0])
                            for (uint8_t x = 0; x < 8; x++)
                            {
                                if (bpp[0] == 32)
                                    framebuffer_offset32[x] = vga_colors[0];
                                else if (bpp[0] == 24)
                                    framebuffer_offset24[x].value = vga_colors[0];
                                else if (bpp[0] == 16)
                                    framebuffer_offset16[x] = vga_colors[0];
                            }
                    }
                    textbuffer_offset += 2;
                }
            }
        }
    }
}