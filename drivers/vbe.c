#include <vbe.h>

vbe_info_t *vbe_info = 0x9500;
vbe_mode_info_t *vbe_mode_info = 0x9000;
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
    vbe_modes->text_framebuffer = kmalloc((vbe_mode_info->width / 8) *
                                          (vbe_mode_info->height / 16) *
                                          sizeof(uint16_t));
    vbe_modes->graphic_framebuffer = kmalloc(vbe_mode_info->width *
                                             vbe_mode_info->height *
                                             round((float)vbe_mode_info->bpp / 8.0));
}

void *vbe_mode_list()
{
    screen_info_t *vesa = kmalloc(sizeof(screen_info_t));
    memset(vesa, 0, sizeof(screen_info_t));
    uint16_t index = 0;
    vbe_mode_info_t *mode_info = vbe_mode_info;
    uint16_t *mode_list = (uint16_t *)vbe_info->video_modes;
    for (uint32_t i = 0; mode_list[i] != 0xffff; i++)
    {
        vbe_get_mode(mode_list[i]);
        if ((mode_info->attributes & 0x90) != 0x90)
            continue;
        if (mode_info->memory_model != 4 && mode_info->memory_model != 6)
            continue;
        index++;
    }
    vesa->video_modes = kmalloc(sizeof(screen_mode_info_t) * (index - 1));
    memset(vesa->video_modes, 0, sizeof(screen_mode_info_t) * (index - 1));
    vesa->video_modes_length = index - 1;
    index = 0;
    for (uint16_t i = 0; mode_list[i] != 0xffff; i++)
    {
        vbe_get_mode(mode_list[i]);
        if ((mode_info->attributes & 0x90) != 0x90)
            continue;
        if (mode_info->memory_model != 4 && mode_info->memory_model != 6)
            continue;
        vesa->video_modes[index].flags = (1 << 1);
        vesa->video_modes[index].framebuffer = mode_info->framebuffer;
        vesa->video_modes[index].mode = mode_list[i];
        vesa->video_modes[index].width = mode_info->width;
        vesa->video_modes[index].height = mode_info->height;
        vesa->video_modes[index].pitch = mode_info->pitch;
        vesa->video_modes[index].bpp = mode_info->bpp;
        index++;
    }
    return vesa;
}

void vbe_install()
{
    uint32_t colors = 0;
    memcpy(0x9500, "VBE2", 4);
    register16_t reg_in = {0};
    register16_t reg_out = {0};
    reg_in.ax = 0x4F00;
    reg_in.di = 0x9500;
    bios32_service(BIOS_GRAPHICS_SERVICE, &reg_in, &reg_out);
    if (reg_out.ax != 0x004F)
        ;
    vbe_modes = vbe_mode_list();
    uint32_t mode_length = vbe_modes->video_modes_length;
    uint32_t mode_xy[2] = {
        0,
        0,
    };
    for (uint32_t i = 0; i < mode_length; i++)
    {
        mode_xy[0] = vbe_modes->video_modes[i].width * vbe_modes->video_modes[i].height;
        if (mode_xy[0] < vbe_modes->video_modes[0].width * vbe_modes->video_modes[0].height)
            continue;
        if (mode_xy[0] >= mode_xy[1] && vbe_modes->video_modes[i].bpp >= 24)
            vbe_modes->current_video_mode = &vbe_modes->video_modes[i];
        mode_xy[1] = vbe_modes->video_modes[i].width * vbe_modes->video_modes[i].height;
    }
    vbe_set_mode(vbe_modes->current_video_mode->mode);
    screen_add(vbe_modes);
}
