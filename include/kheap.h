#ifndef KHEAP_H
#define KHEAP_H
#include <kernel.h>

extern struct heap *kheap;

void kheap_install();
uint32_t kmalloc_int(uint32_t size, uint16_t align, uint32_t *phys);
uint32_t kmalloc_a(uint32_t size, uint16_t align);
uint32_t kmalloc_p(uint32_t size, uint32_t *phys);
uint32_t kmalloc_ap(uint32_t size, uint16_t align, uint32_t *phys);
uint32_t kmalloc(uint32_t size);
uint32_t krealloc(void *ptr, uint32_t size);
uint32_t kclone(void *ptr);
void kfree(void *ptr);
#endif