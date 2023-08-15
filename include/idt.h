#pragma once
#ifndef IDT_H
#define IDT_H
#include <ctype.h>

typedef struct idt_entry_t
{
    uint16_t base_lo;
    uint16_t sel;
    uint8_t always0;
    uint8_t flags;
    uint16_t base_hi;
} __attribute__((packed)) idt_entry_t;

typedef struct idtr_t
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idtr_t;

typedef struct registers
{
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} register_t;

typedef void (*handler_t)(register_t *);

extern idt_entry_t idt[256];
extern idtr_t idtr;

void idt_set_entry(uint8_t num, void *base, uint16_t sel, uint8_t flags);
void idt_install();
#endif