#include <canvas.h>

const uint8_t bayer_dither_16x16[256] = {
    0, 191, 48, 239, 12, 203, 60, 251, 3, 194, 51, 242, 15, 206, 63, 254,
    127, 64, 175, 112, 139, 76, 187, 124, 130, 67, 178, 115, 142, 79, 190, 127,
    32, 223, 16, 207, 44, 235, 28, 219, 35, 226, 19, 210, 47, 238, 31, 222,
    159, 96, 143, 80, 171, 108, 155, 92, 162, 99, 146, 83, 174, 111, 158, 95,
    8, 199, 56, 247, 4, 195, 52, 243, 11, 202, 59, 250, 7, 198, 55, 246,
    135, 72, 183, 120, 131, 68, 179, 116, 138, 75, 186, 123, 134, 71, 182, 119,
    40, 231, 24, 215, 36, 227, 20, 211, 43, 234, 27, 218, 39, 230, 23, 214,
    167, 104, 151, 88, 163, 100, 147, 84, 170, 107, 154, 91, 166, 103, 150, 87,
    2, 193, 50, 241, 14, 205, 62, 253, 1, 192, 49, 240, 13, 204, 61, 252,
    129, 66, 177, 114, 141, 78, 189, 126, 128, 65, 176, 113, 140, 77, 188, 125,
    34, 225, 18, 209, 46, 237, 30, 221, 33, 224, 17, 208, 45, 236, 29, 220,
    161, 98, 145, 82, 173, 110, 157, 94, 160, 97, 144, 81, 172, 109, 156, 93,
    10, 201, 58, 249, 6, 197, 54, 245, 9, 200, 57, 248, 5, 196, 53, 244,
    137, 74, 185, 122, 133, 70, 181, 118, 136, 73, 184, 121, 132, 69, 180, 117,
    42, 233, 26, 217, 38, 229, 22, 213, 41, 232, 25, 216, 37, 228, 21, 212,
    169, 106, 153, 90, 165, 102, 149, 86, 168, 105, 152, 89, 164, 101, 148, 85};

uint8_t canvas_text_mask[8] = {
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

uint8_t canvas_rgb_grayscale(uint32_t x, uint32_t y, uint32_t c)
{
    uint8_t r = (c >> 16) & 0xff, g = (c >> 8) & 0xff, b = c & 0xff;
    uint16_t bayer_id = (y & 15) * 16 + (x & 15);
    return r;
}

uint32_t canvas_rgb_dithering(uint8_t ncolors, uint32_t x, uint32_t y, uint32_t c)
{
    uint8_t bpc = ncolors;
    uint16_t divider = 255 / bpc;
    uint8_t r = (c >> 16) & 0xff, g = (c >> 8) & 0xff, b = c & 0xff;
    uint16_t bayer_id = (y & 15) * 16 + (x & 15);
    uint32_t corr = (bayer_dither_16x16[bayer_id] / bpc);
    uint32_t i1 = (b + corr) / divider;
    b = clamp_int(i1 * divider, 0, 0xff);
    uint32_t i2 = (g + corr) / divider;
    g = clamp_int(i2 * divider, 0, 0xff);
    uint32_t i3 = (r + corr) / divider;
    r = clamp_int(i3 * divider, 0, 0xff);
    uint24_t rgb = {.rgb.b = r, .rgb.g = g, .rgb.r = b};
    return rgb.value;
}

uint16_t canvas_rgb_24to16(uint32_t c)
{
    uint8_t b = (uint16_t)(((c & 0xff) >> 3) & 0x1F);
    uint8_t g = (uint16_t)((((c >> 8) & 0xff) >> 2) & 0x3F);
    uint8_t r = (uint16_t)((((c >> 16) & 0xff) >> 3) & 0x1F);
    return (r << (16 - 5)) | (g << (16 - (5 + 6))) | b << (16 - (5 + 6 + 5));
}

canvas_t *canvas_create(uint32_t video_memory, uint16_t width, uint16_t height, uint8_t bpp)
{
    canvas_t *ret = kmalloc(sizeof(canvas_t) + (width * height * sizeof(uint32_t)));
    if (!ret)
        return (canvas_t *)NULL;
    memset(ret, 0, sizeof(canvas_t));
    ret->width = width, ret->height = height, ret->bpp = bpp;
    ret->pixelstride = sizeof(uint32_t);
    ret->pitch = ret->width * sizeof(uint32_t);
    ret->video_buffer_size = ret->pitch * ret->height;
    ret->video_buffer = (uint32_t)ret + sizeof(canvas_t);
    memset(ret->video_buffer, 0, ret->video_buffer_size);
    ret->video_memory = video_memory;
    return ret;
}

void canvas_fillscreen(canvas_t *canvas, uint32_t color)
{
    if (!canvas || !canvas->video_buffer)
        return;
    memset(canvas->video_buffer, color, canvas->video_buffer_size);
}

void canvas2framebuffer(canvas_t *canvas)
{
    if (!canvas || (!canvas->video_memory && !canvas->video_buffer))
        return;
    uint32_t *video_buffer32 = canvas->video_buffer;
    switch (canvas->bpp)
    {
    case 32:
        uint32_t *video_memory32 = canvas->video_memory;
        for (uint16_t y = 0; y < canvas->height; y++)
            for (uint16_t x = 0; x < canvas->width; x++)
                *video_memory32++ = *video_buffer32++;
        break;
    case 24:
        uint24_t *video_memory24 = canvas->video_memory;
        for (uint16_t y = 0; y < canvas->height; y++)
            for (uint16_t x = 0; x < canvas->width; x++)
                (*video_memory24++).value = *video_buffer32++;
        break;
    case 16:
        uint16_t *video_memory16 = canvas->video_memory;
        for (uint16_t y = 0; y < canvas->height; y++)
            for (uint16_t x = 0; x < canvas->width; x++)
                *video_memory16++ = canvas_rgb_24to16(*video_buffer32++);
        break;
    default:
        break;
    }
}

void canvas_copy(canvas_t *canvas, canvas_t *canvas_src, float x, float y)
{
    if (!canvas || !canvas->video_buffer)
        return;
    if (!canvas_src || !canvas_src->video_buffer)
        return;
    int64_t intx = round(x), inty = round(y);
    uint32_t intx_precalc = ((intx > 0 ? intx : 0) << 2);
    uint32_t inty_precalc = (canvas->pitch * clamp_int(inty, 0, canvas->height));
    uint32_t *buffer0 = canvas->video_buffer + inty_precalc, *buffer1 = canvas_src->video_buffer;
    for (uint16_t sy = 0; sy < canvas_src->height; sy++)
    {
        if (inty + sy < 0 || inty + sy >= canvas->height)
            continue;
        uint32_t *buffer0X = (uint32_t)buffer0 + intx_precalc;
        uint32_t *buffer1X = (uint32_t)buffer1;
        for (uint16_t sx = 0; sx < canvas_src->width; sx++)
        {
            if (intx + sx < 0 || intx + sx >= canvas->width)
                continue;
            *buffer0X++ = *buffer1X++;
        }
        buffer0 = (uint32_t)buffer0 + canvas->pitch;
        buffer1 = (uint32_t)buffer1 + canvas_src->pitch;
    }
}

void canvas_scroll(canvas_t *canvas, uint16_t y)
{
    if (!canvas || !canvas->video_buffer)
        return;
    if (y >= canvas->height)
        return;
    uint32_t pos0 = y * canvas->pitch;
    uint32_t pos1 = (canvas->pitch * canvas->height) - pos0;
    memcpy(canvas->video_buffer, canvas->video_buffer + pos0, pos1);
    memset(canvas->video_buffer + pos1, 0, pos0);
}

void canvas_putpixel(canvas_t *canvas, float x, float y, uint32_t rgb)
{
    if (!canvas || !canvas->video_buffer)
        return;
    int64_t intx = round(x), inty = round(y);
    if (intx < 0 || intx >= canvas->width)
        return;
    if (inty < 0 || inty >= canvas->height)
        return;
    uint32_t *video_buffer = canvas->video_buffer + ((inty * canvas->pitch) + (intx << 2));
    *video_buffer = blend_colors(*video_buffer, rgb);
}

void canvas_drawrect(canvas_t *canvas, float x, float y, float width, float height, uint32_t rgb)
{
    if (!canvas || !canvas->video_buffer)
        return;
    if (!width || !height)
        return;
    int64_t intwidth = round(width), intheight = round(height), intx = round(x), inty = round(y);
    uint32_t intx_precalc = ((intx > 0 ? intx : 0) << 2);
    uint32_t *video_buffer = canvas->video_buffer + (canvas->pitch * clamp_int(inty, 0, canvas->height));
    for (uint16_t dy = 0; dy < intheight; dy++)
    {
        if (inty + dy < 0 || inty + dy >= canvas->height)
            continue;
        uint32_t *video_bufferX = (uint32_t)video_buffer + intx_precalc;
        for (uint16_t dx = 0; dx < intwidth; dx++)
        {
            if (intx + dx < 0 || intx + dx >= canvas->width)
                continue;
            uint32_t value = *video_bufferX;
            *video_bufferX++ = blend_colors(value, rgb);
        }
        video_buffer = (uint32_t)video_buffer + canvas->pitch;
    }
}

void canvas_drawchar(canvas_t *canvas, uint8_t chr, float x, float y, float width, float height, uint32_t rgb)
{
    if (!canvas || !canvas->video_buffer)
        return;
    if (!width || !height)
        return;
    float scale_x = width / (float)font[0];
    float scale_y = height / (float)font[1];
    uint8_t *gylph = font + 2 + (chr * font[1]);
    int64_t intwidth = round(width), intheight = round(height), intx = round(x), inty = round(y);
    uint32_t intx_precalc = ((intx > 0 ? intx : 0) << 2);
    uint32_t *video_buffer = canvas->video_buffer + (canvas->pitch * clamp_int(inty, 0, canvas->height));
    for (uint16_t dy = 0; dy < intheight; dy++)
    {
        if (inty + dy < 0 || inty + dy >= canvas->height)
            continue;
        uint32_t *video_bufferX = (uint32_t)video_buffer + intx_precalc;
        uint16_t y_gylph = round(dy / scale_y);
        for (uint16_t dx = 0; dx < intwidth; dx++)
        {
            if (intx + dx < 0 || intx + dx >= canvas->width)
                continue;
            uint32_t value = *video_bufferX;
            if (gylph[y_gylph] & canvas_text_mask[round(dx / scale_x)])
                *video_bufferX++ = blend_colors(value, rgb);
            else
                *video_bufferX++;
        }
        video_buffer = (uint32_t)video_buffer + canvas->pitch;
    }
}

void canvas_drawstring(canvas_t *canvas, uint8_t *string, float x, float y, float width, float height, uint32_t rgb, bool breakline)
{
    if (!canvas || !canvas->video_buffer)
        return;
    if (!width || !height)
        return;
    if (!string || !*string)
        return;
    while (*string)
    {
        switch (*string)
        {
        case '\r':
            break;
        case '\0':
            break;
        case '\t':
            x += width * 4;
            if (breakline)
                if (x * width >= canvas->width)
                    y += height, x = 0;
            break;
        case '\n':
            x = 0;
            y += height;
            if (breakline)
                if (x * width >= canvas->width)
                    y += height, x = 0;
            break;
        default:
            if (breakline)
                if (x * width >= canvas->width)
                    y += height, x = 0;
            canvas_drawchar(canvas, *string, x, y, width, height, rgb);
            x += width;
            break;
        }
        *string++;
    }
}

void canvas_drawimage(canvas_t *canvas, uint32_t *data,
                      float x, float y, float width, float height,
                      float sx, float sy, float swidth, float sheight)
{
    if (!canvas || !canvas->video_buffer)
        return;
    if (!width || !height)
        return;
    if (!swidth || !sheight)
        return;
    if (!data)
        return;
    uint16_t sswidth = *data++, ssheight = *data++;
    uint16_t sspitch = sswidth << 2;
    float scale_x = width / (float)sswidth;
    if (swidth < sswidth || swidth)
        scale_x = width / (float)swidth;
    float scale_y = height / (float)ssheight;
    if (sheight < ssheight || sheight)
        scale_y = height / (float)sheight;
    if (swidth > sswidth)
        swidth = sswidth;
    if (sheight > ssheight)
        sheight = ssheight;
    int64_t intwidth = round(width), intheight = round(height), intx = round(x), inty = round(y);
    int64_t intswidth = round(swidth), intsheight = round(sheight), intsx = round(sx), intsy = round(sy);
    uint32_t intx_precalc = ((intx > 0 ? intx : 0) << 2);
    uint32_t *video_buffer = canvas->video_buffer + (canvas->pitch * clamp_int(inty, 0, canvas->height));
    for (int16_t dy = 0, sy = 0; dy < intheight; dy++)
    {
        if (inty + dy < 0 || inty + dy >= canvas->height)
            continue;
        sy = round(dy / scale_y);
        uint32_t *video_bufferX = (uint32_t)video_buffer + intx_precalc;
        uint32_t *dataX = (uint32_t)data + ((sy + intsy) * sspitch);
        for (int16_t dx = 0, sx = 0; dx < intwidth; dx++)
        {
            if (intx + dx < 0 || intx + dx >= canvas->width)
                continue;
            sx = round(dx / scale_x);
            uint32_t value = *video_bufferX;
            *video_bufferX++ = blend_colors(value,
                                            (sx + intsx < 0 || sx + intsx >= sswidth || sy + intsy < 0 || sy + intsy >= ssheight) ? 0
                                                                                                                                  : dataX[sx + intsx]);
        }
        video_buffer = (uint32_t)video_buffer + canvas->pitch;
    }
}