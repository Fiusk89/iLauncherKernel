#pragma once
#ifndef IO_H
#define IO_H
#include <kernel.h>

void outb(uint16_t port, uint8_t data);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
void outw(uint16_t port, uint16_t data);
uint32_t inl(uint16_t port);
void outl(uint16_t port, uint32_t data);
void io_wait();
void mm_outb(void *port, uint8_t data);
uint8_t mm_inb(void *port);
void mm_outw(void *port, uint16_t data);
uint16_t mm_inw(void *port);
void mm_outl(void *port, uint32_t data);
uint32_t mm_inl(void *port);
#endif