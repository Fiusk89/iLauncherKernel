#include <vbe.h>

vbe_info_t *vbe_info = (vbe_info_t *)0x9500;
vbe_mode_info_t *vbe_mode_info = (vbe_mode_info_t *)0x9000;
screen_info_t *vbe_modes;
uint64_t *vbe_memory = 0;
uint16_t vbe_current_mode;
uint32_t image_cur = 0, image_max = 0;
uint32_t **image = NULL;

void vga_disable()
{
    outb(VGA_CRTC_INDEX, 0x01);
    outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) & 0x20);
}

void vbe_get_mode(uint16_t mode)
{
    if (mode == 0xffff)
        return;
    register16_t reg_in = {0};
    register16_t reg_out = {0};
    reg_in.ax = 0x4F01;
    reg_in.cx = mode;
    reg_in.di = 0x9000;
    bios32_service(BIOS_GRAPHICS_SERVICE, &reg_in, &reg_out);
}

uint8_t bpp_check(uint8_t bpp1, uint8_t bpp2)
{
    if (bpp1 == 32)
        if (bpp2 == 24)
            return bpp2;
    return bpp1;
}

void vbe_set_mode(uint16_t mode)
{
    if (mode == 0xffff)
        return;
    register16_t reg_in = {0};
    register16_t reg_out = {0};
    reg_in.ax = 0x4F02;
    reg_in.bx = mode;
    bios32_service(BIOS_GRAPHICS_SERVICE, &reg_in, &reg_out);
    vbe_current_mode = mode;
    vbe_get_mode(mode);
    uint32_t fbSize = vbe_mode_info->width * vbe_mode_info->height * round((float)vbe_mode_info->bpp / 8.0);
    for (uint32_t i = vbe_mode_info->framebuffer; i < vbe_mode_info->framebuffer + fbSize; i += 0x1000)
    {
        page_alloc_frame(kernel_directory, i, i, 1, 1);
    }
    kfree(vbe_modes->text_framebuffer);
    kfree(vbe_modes->graphic_framebuffer);
    vbe_modes->text_framebuffer = (uint16_t *)kmalloc((vbe_mode_info->width / 8) *
                                                      (vbe_mode_info->height / 16) *
                                                      sizeof(uint16_t));
    vbe_modes->graphic_framebuffer = (void *)kmalloc(vbe_mode_info->width *
                                                     vbe_mode_info->height *
                                                     round((float)vbe_mode_info->bpp / 8.0));
}

void *vbe_mode_list()
{
    screen_info_t *vesa = (screen_info_t *)kmalloc(sizeof(screen_info_t));
    memset(vesa, 0, sizeof(screen_info_t));
    uint16_t index = 0;
    vbe_mode_info_t *mode_info = vbe_mode_info;
    uint16_t *mode_list = (uint16_t *)vbe_info->video_modes;
    vesa->video_modes = (screen_mode_info_t *)kmalloc(sizeof(screen_mode_info_t));
    memset(vesa->video_modes, 0, sizeof(screen_mode_info_t));
    screen_mode_info_t *tmp = vesa->video_modes;
    for (uint16_t i = 0; mode_list[i] != 0xffff; i++)
    {
        vbe_get_mode(mode_list[i]);
        if ((mode_info->attributes & 0x90) != 0x90)
            continue;
        if (mode_info->memory_model != 4 && mode_info->memory_model != 6)
            continue;
        tmp->flags = (1 << 1);
        tmp->framebuffer = mode_info->framebuffer;
        tmp->mode = mode_list[i];
        tmp->width = mode_info->width;
        tmp->height = mode_info->height;
        tmp->pitch = mode_info->pitch;
        tmp->twidth = mode_info->width / 8;
        tmp->theight = mode_info->height / 16;
        tmp->tpitch = sizeof(uint16_t) * (mode_info->width / 8);
        tmp->bpp = mode_info->bpp;
        tmp->next = (screen_mode_info_t *)kmalloc(sizeof(screen_mode_info_t));
        memset(tmp->next, 0, sizeof(screen_mode_info_t));
        tmp->next->prev = tmp;
        tmp = tmp->next;
    }
    return vesa;
}

void vbe_install()
{
    uint32_t colors = 0;
    memcpy((void *)0x9500, "VBE2", 4);
    register16_t reg_in = {0};
    register16_t reg_out = {0};
    reg_in.ax = 0x4F00;
    reg_in.di = 0x9500;
    bios32_service(BIOS_GRAPHICS_SERVICE, &reg_in, &reg_out);
    if (reg_out.ax != 0x004F)
        ;
    vbe_modes = vbe_mode_list();
    uint32_t mode_length = vbe_modes->video_modes_length;
    uint32_t mode_xy = 0;
    screen_mode_info_t *tmp = vbe_modes->video_modes;
    while (tmp->next)
    {
        if (tmp->height * tmp->pitch >= mode_xy && tmp->bpp >= 24)
            vbe_modes->current_video_mode = tmp, mode_xy = tmp->height * tmp->pitch;
        tmp = tmp->next;
    }
    vbe_set_mode(vbe_modes->current_video_mode->mode);
    screen_add(vbe_modes);
}
