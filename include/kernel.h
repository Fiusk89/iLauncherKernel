#ifndef KERNEL_H
#define KERNEL_H
#define KERNEL_BASE_ADDRESS 0xC0000000
#define KERNEL_V2P(a) ((uint32_t)(a) & ~KERNEL_BASE_ADDRESS)
#define KERNEL_P2V(a) ((uint32_t)(a) | KERNEL_BASE_ADDRESS)
#define KERNEL_ALIGN(address, align) align > 1 ? (address + (align - 1) & ~(align - 1)) : address
#define RGB2GRAY(c) (((((c) >> 16) & 0xff) + (((c) >> 8) & 0xff) + ((c)&0xff)) / 3)
#include <ctype.h>
#include <multiboot.h>
#include <screen.h>
#include <rle.h>
#include <stdlib.h>
#include <string.h>
#include <blend.h>
#include <math.h>
#include <io.h>
#include <fs.h>
#include <gdt.h>
#include <tss.h>
#include <idt.h>
#include <isr.h>
#include <irq.h>
#include <syscall.h>
#include <canvas.h>
#include <cpuid.h>
#include <cpu.h>
#include <apic.h>
#include <bios32.h>
#include <pic.h>
#include <fonts.h>
#include <pc_speaker.h>
#include <pit.h>
#include <acpi.h>
#include <smp.h>
#include <vbe.h>
#include <vga.h>
#include <mouse.h>
#include <textmode.h>
#include <stdarg.h>
#include <stdio.h>
#include <fpu.h>
#include <keyboard.h>
#include <heap.h>
#include <kheap.h>
#include <path.h>
#include <ilfs.h>
#include <page.h>
#include <task.h>
#include <pci.h>
#include <image.h>
#include <ide.h>
#include <window.h>

#define KB 1024
#define MB (1024 * 1024)
#define GB (1024 * 1024 * 1024)

static void disable()
{
    for (uint8_t i = 0; i < 15; i++)
        asm volatile("cli");
}

static void enable()
{
    asm volatile("sti");
    asm volatile("int $0x20");
}

void poweroff();
void reboot();

extern uint32_t placement_address;
#endif