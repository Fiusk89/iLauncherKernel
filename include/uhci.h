#pragma once
#ifndef UHCI_H
#define UHCI_H
#include <kernel.h>

typedef struct uhci
{
    uint8_t irq;
    uint32_t *frame_list;
    uint16_t command;
    uint16_t status;
    uint16_t interrupt_enable;
    uint16_t frame_number;
    uint32_t frame_list_base_address;
    uint8_t start_frame_modify;
    uint16_t port1, port2;
    struct uhci *prev;
    struct uhci *next;
} __attribute__((packed)) uhci_t;

void uhci_install();
#endif