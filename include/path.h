#ifndef PATH_H
#define PATH_H
#include <kernel.h>

uint8_t **path(uint8_t *file);
void path_free(uint8_t **p);
#endif