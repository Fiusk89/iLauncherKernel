#ifndef PIC_H
#define PIC_H
#include <kernel.h>
#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1
#define PIC_IRQ_BASE 0x20
#define PIC_IRQ_CASCADE 0x02
#define PIC_EOI 0x20
#define PIC_SPEC_EOI 0x60    /* Specific End-of-interrupt command */
#define PIC_READ_IRR 0x0A    /* OCW3 irq ready next CMD read */
#define PIC_READ_ISR 0x0B    /* OCW3 irq service next CMD read */
#define PIC_CASCADE 0x02     /* IRQ in the master for slave IRQs */
#define ICW1_ICW4 0x01       /* Indicates that ICW4 will be present */
#define ICW1_SINGLE 0x02     /* Single (cascade) mode */
#define ICW1_INTERVAL4 0x04  /* Call address interval 4 (8) */
#define ICW1_LEVEL 0x08      /* Level triggered (edge) mode */
#define ICW1_INIT 0x10       /* Initialization - required! */
#define ICW4_8086 0x01       /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO 0x02       /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE 0x08  /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C /* Buffered mode/master */
#define ICW4_SFNM 0x10       /* Special fully nested (not) */

uint16_t pic_read_irq_register(uint8_t ocw3);
void pic_send_eoi(uint8_t irq, uint8_t mask);
void pic_mask_irq(uint8_t irq);
void pic_unmask_irq(uint8_t irq);
void pic_install();
void pic_unmask_all();
void pic_mask_all();
#endif