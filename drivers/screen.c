#include <screen.h>

screen_info_t *start_screen;
screen_info_t *current_screen;
uint8_t screen_mode = 0x00;
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
            sleep(500);
        if (current_screen->current_video_mode->flags & (1 << 1))
        {
            uint16_t width = current_screen->current_video_mode->width / 8;
            uint16_t height = current_screen->current_video_mode->height / 16;
            uint8_t bpp = round((float)current_screen->current_video_mode->bpp / 8.0f);
            for (uint16_t ty = 0; ty < height; ty++)
            {
                for (uint16_t tx = 0; tx < width; tx++)
                {
                    if (current_task->active_time - saved_timer >= 150)
                        screen_cursor = screen_cursor ? false : true, saved_timer = current_task->active_time;
                    uint8_t *textbuffer_offset = (uint8_t *)(current_screen->text_framebuffer +
                                                             (sizeof(uint16_t) * (ty * width + tx)));
                    uint8_t *gylph = font + 2 + (textbuffer_offset[0] * font[1]);
                    for (uint8_t y = 0; y < 16; y++)
                    {
                        for (uint8_t x = 0; x < 8; x++)
                        {
                            if (!textbuffer_offset[0])
                                break;
                            uint24_t *framebuffer_offset = (uint24_t *)(current_screen->current_video_mode->framebuffer +
                                                                        (((ty * 16) + y) *
                                                                             current_screen->current_video_mode->width +
                                                                         ((tx * 8) + x)) *
                                                                            bpp);
                            if (gylph[y] & screen_text_mask[x])
                                framebuffer_offset->value = vga_colors[textbuffer_offset[1] & 0x0f];
                            else
                                framebuffer_offset->value = vga_colors[(textbuffer_offset[1] >> 4) & 0x0f];
                        }
                    }
                    textbuffer_offset = (uint8_t *)(current_screen->text_framebuffer +
                                                    (sizeof(uint16_t) * (screen_cursor_position * 8)));
                    if (screen_cursor)
                        for (uint8_t y = screen_cursor_start; y < screen_cursor_end; y++)
                        {
                            if (screen_cursor_position >
                                current_screen->current_video_mode->pitch *
                                    current_screen->current_video_mode->height)
                                break;
                            uint24_t *framebuffer_offset = (uint24_t *)(current_screen->current_video_mode->framebuffer +
                                                                        (screen_cursor_position * 8) +
                                                                        (current_screen->current_video_mode->pitch * y));
                            for (uint8_t x = 0; x < 8; x++)
                            {
                                framebuffer_offset[x].value = vga_colors[textbuffer_offset[1] & 0x0f];
                            }
                        }
                }
            }
        }
    }
}