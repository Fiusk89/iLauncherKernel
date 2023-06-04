#ifndef CANVAS_H
#define CANVAS_H
#include <kernel.h>

typedef struct canvas
{
	uint32_t video_memory, video_buffer, video_buffer_size;
	uint16_t width;
	uint16_t height;
	uint16_t pixelstride, pitch;
	uint16_t bpp;
} canvas_t;

canvas_t *canvas_create(uint32_t video_memory, uint16_t width, uint16_t height, uint8_t bpp);
void canvas2framebuffer(canvas_t *canvas);
void canvas_fillscreen(canvas_t *canvas, uint32_t color);
void canvas_scroll(canvas_t *canvas, uint16_t y);
void canvas_copy(canvas_t *canvas, canvas_t *canvas_src, float x, float y);
void canvas_putpixel(canvas_t *canvas, float x, float y, uint32_t rgb);
void canvas_drawrect(canvas_t *canvas, float x, float y, float width, float height, uint32_t rgb);
void canvas_drawimage(canvas_t *canvas, uint32_t *data,
                      float x, float y, float width, float height,
                      float sx, float sy, float swidth, float sheight);
void canvas_drawchar(canvas_t *canvas, uint8_t chr, float x, float y, float width, float height, uint32_t rgb);
void canvas_drawstring(canvas_t *canvas, uint8_t *string, float x, float y, float width, float height, uint32_t rgb, bool breakline);
#endif