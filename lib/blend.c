#include <blend.h>

uint32_t blend_colors(uint32_t color1, uint32_t color2)
{
  uint8_t alpha = color2 >> 24;
  if (!alpha)
    return color1;
  uint32_t rb = color1 & 0xff00ff;
  uint32_t g = color1 & 0x00ff00;
  rb += ((color2 & 0xff00ff) - rb) * alpha >> 8;
  g += ((color2 & 0x00ff00) - g) * alpha >> 8;
  return (rb & 0xff00ff) | (g & 0xff00);
}