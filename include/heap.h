#ifndef HEAP_H
#define HEAP_H
#include <kernel.h>

#define HEAP_SIGNATURE 0xF0FF0F0F000F0F0F

typedef struct heap
{
    uint64_t signature;
    uint64_t start, end, max;
    uint8_t is_kernel, is_writable;
} heap_t;

typedef struct heap_node
{
    uint64_t signature;
    uint8_t is_free;
    uint64_t size;
    uint16_t align;
    struct heap_node *prev;
    struct heap_node *next;
} heap_node_t;

heap_t *heap_create(uint64_t start, uint64_t end, uint64_t max, uint8_t is_kernel, uint8_t is_writable);
uint64_t heap_get_free_size(heap_t *heap);
uint64_t heap_get_used_size(heap_t *heap);
void *heap_malloc(heap_t *heap, uint64_t size, uint16_t align);
void *heap_mrealloc(heap_t *heap, void *ptr, uint64_t size);
void *heap_mclone(heap_t *heap, void *ptr);
void heap_mfree(heap_t *heap, void *ptr);
#endif