#pragma once
#ifndef GDT_H
#define GDT_H
#include <kernel.h>

typedef struct gdt_entry_t
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct gdtr_t
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) gdtr_t;

extern gdt_entry_t gdt[9];
extern gdtr_t gdtr;

void gdt_set_entry(uint8_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
void gdt_install();
#endif