#pragma once
#ifndef PCI_H
#define PCI_H
#include <kernel.h>

typedef struct pci
{
    uint16_t vendor, device;
    uint8_t class, subclass, interface;
    uint8_t bus, slot, function;
    struct pci *prev;
    struct pci *next;
} pci_t;

uint16_t pci_read(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset);
void pci_write(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint32_t value);
uint32_t pci_read_bar_address(uint8_t bus, uint8_t slot, uint8_t function, uint8_t bar);
void pci_write_bar_address(uint8_t bus, uint8_t slot, uint8_t function, uint8_t bar, uint32_t value);
uint32_t *pci_find_devices(uint8_t length, uint8_t class, uint8_t subclass, uint8_t interface);
void pci_install();
#endif