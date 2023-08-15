#pragma once
#ifndef APIC_H
#define APIC_H
#include <kernel.h>

extern uint32_t lapic_addr;
extern uint32_t ioapic_addr;

void ioapic_set_entry(uint32_t base, uint8_t index, uint64_t data);
void ioapic_install();
void lapic_cpu_enable(uint32_t apic_id);
void lapic_cpu_start(uint32_t apic_id, uint32_t vector);
void lapic_end();
uint32_t lapic_id();
void lapic_install();
#endif