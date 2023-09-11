#pragma once
#ifndef UHCI_H
#define UHCI_H
#include <kernel.h>
#define UHCI_STATUS_HCHALTED (1 << 5)
#define UHCI_STATUS_HC_PROCESS_ERROR (1 << 4)
#define UHCI_STATUS_HOST_SYSTEM_ERROR (1 << 3)
#define UHCI_STATUS_RESUME_DETECT (1 << 2)
#define UHCI_STATUS_USB_ERROR (1 << 1)
#define UHCI_STATUS_USBINT (1 << 0)
#define UHCI_PORT_WRITE_MASK 0x124E
#define UHCI_PORT_CONNECTION (1 << 0)
#define UHCI_PORT_CONNECTION_CHANGE (1 << 1)
#define UHCI_PORT_ENABLE (1 << 2)
#define UHCI_PORT_ENABLE_CHANGE (1 << 3)
#define UHCI_PORT_LS (3 << 4)
#define UHCI_PORT_RD (1 << 6)
#define UHCI_PORT_LSDA (1 << 8)
#define UHCI_PORT_RESET (1 << 9)
#define UHCI_PORT_SUSP (1 << 12)
#define UHCI_PORT_RWC (PORT_CONNECTION_CHANGE | PORT_ENABLE_CHANGE)
#define UHCI_CMD_RUN_STOP (1 << 0)
#define UHCI_CMD_HOST_CONTROLLER_RESET (1 << 1)
#define UHCI_CMD_GLOBAL_RESET (1 << 2)
#define UHCI_CMD_ENTER_GLOBAL_SUSPEND_RESUME (1 << 3)
#define UHCI_CMD_FORCE_GLOBAL_RESUME (1 << 4)
#define UHCI_CMD_SOFTWARE_DEBUG (1 << 5)
#define UHCI_CMD_CONFIGURE_FLAG (1 << 6)
#define UHCI_CMD_MAX_PACKET (1 << 7)
#define UHCI_QUEUE_Q128 0
#define UHCI_QUEUE_Q64 1
#define UHCI_QUEUE_Q32 2
#define UHCI_QUEUE_Q16 3
#define UHCI_QUEUE_Q8 4
#define UHCI_QUEUE_Q4 5
#define UHCI_QUEUE_Q2 6
#define UHCI_QUEUE_Q1 7
#define UHCI_QUEUE_QControl 8
#define UHCI_QUEUE_QBulk 9
#define UHCI_QUEUE_STRUCT_PTR_MASK 0xfffffff0
#define UHCI_QUEUE_STRUCT_Q 0x02
#define UHCI_QUEUE_STRUCT_T 0x01
#define UHCI_QUEUE_LEN 10

typedef struct uhci_queue
{
    uint32_t horizontal;
    uint32_t vertical;
    struct uhci_queue *next;
    struct uhci_queue *parent;
} __attribute__((packed)) uhci_queue_t;

typedef struct uhci
{
    uint8_t irq;
    uint32_t frame_list_physical;
    uint32_t *frame_list;
    uint32_t stack_list_physical;
    uhci_queue_t *stack_list;
    uint32_t command;                 // CMD
    uint32_t status;                  // STS
    uint32_t interrupt_enable;        // INTR
    uint32_t frame_number;            // FRNUM
    uint32_t frame_list_base_address; // FRBASEADD
    uint32_t start_frame_modify;      // SOFMOD
    uint32_t port;                    // PORT
    uint32_t legacy_support;          // LEGSUP
    struct uhci *prev;
    struct uhci *next;
} __attribute__((packed)) uhci_t;

void uhci_install();
#endif