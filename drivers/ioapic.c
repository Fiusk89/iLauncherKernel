#include <apic.h>

uint32_t ioapic_addr;

#define IOAPIC_REGSEL 0x00
#define IOAPIC_REGWIN 0x10
#define IOAPIC_ID 0x00
#define IOAPIC_VER 0x01
#define IOAPIC_ARB 0x02
#define IOAPIC_REDTBL_BASE 0x10
#define IOAPIC_REDTBL_INTVEC_MAKE(i) ((i)&0xFF)
#define IOAPIC_REDTBL_DELMOD_FIXED (0x0 << 8)
#define IOAPIC_REDTBL_DELMOD_LOWEST (0x1 << 8)
#define IOAPIC_REDTBL_DELMOD_SMI (0x2 << 8)
#define IOAPIC_REDTBL_DELMOD_NMI (0x4 << 8)
#define IOAPIC_REDTBL_DELMOD_INIT (0x5 << 8)
#define IOAPIC_REDTBL_DELMOD_EXTINT (0x7 << 8)
#define IOAPIC_REDTBL_DESTMOD_PHYSICAL (0 << 10)
#define IOAPIC_REDTBL_DESTMOD_LOGICAL (1 << 10)
#define IOAPIC_REDTBL_DELIVS_IDLE (0 << 11)
#define IOAPIC_REDTBL_DELIVS_SEND_PENDING (1 << 11)
#define IOAPIC_REDTBL_INTPOL_HIGH_ACTIVE (0 << 12)
#define IOAPIC_REDTBL_INTPOL_LOW_ACTIVE (1 << 12)
#define IOAPIC_REDTBL_REMOTEIRR_REC_EOI (0 << 13)
#define IOAPIC_REDTBL_REMOTEIRR_ACCEPTING (1 << 13)
#define IOAPIC_REDTBL_TRIGGERMOD_EDGE (0 << 14)
#define IOAPIC_REDTBL_TRIGGERMOD_LEVEL (1 << 14)
#define IOAPIC_REDTBL_INTMASK_UNMASKED (0 << 15)
#define IOAPIC_REDTBL_INTMASK_MASKED (1 << 15)
#define IOAPIC_REDTBL_DESTINATION_MAKE(i, f) ((((uint64_t)i & 0xFF) | (uint64_t)f) << 56)
#define IOAPIC_REDTBL_DESTINATION_FLAG_PHYSICAL (0 << 10)
#define IOAPIC_REDTBL_DESTINATION_FLAG_LOGICAL (1 << 10)
#define IOAPIC_REDTBL_MASK_INTVEC (0xFF)
#define IOAPIC_REDTBL_MASK_DELMOD (7 << 8)
#define IOAPIC_REDTBL_MASK_DESTMOD (1 << 10)
#define IOAPIC_REDTBL_MASK_DELIVS (1 << 11)
#define IOAPIC_REDTBL_MASK_INTPOL (1 << 12)
#define IOAPIC_REDTBL_MASK_REMOTEIRR (1 << 13)
#define IOAPIC_REDTBL_MASK_TRIGGERMOD (1 << 14)
#define IOAPIC_REDTBL_MASK_INTMASK (1 << 15)
#define IOAPIC_REDTBL_MASK_RESERVED (0xFFFFFFFFFFC << 16)
#define IOAPIC_REDTBL_MASK_DESTINATION (0xFF << 55)

static uint32_t ioapic_read(uint32_t base, uint8_t reg)
{
    mm_outl((void *)(base + IOAPIC_REGSEL), reg);
    return mm_inl((void *)(base + IOAPIC_REGWIN));
}

static void ioapic_write(uint32_t base, uint8_t reg, uint32_t val)
{
    mm_outl((void *)(base + IOAPIC_REGSEL), reg);
    mm_outl((void *)(base + IOAPIC_REGWIN), val);
}

void ioapic_set_entry(uint32_t base, uint8_t index, uint64_t data)
{
    *((volatile uint32_t *)(base + IOAPIC_REGSEL)) = IOAPIC_REDTBL_BASE + index * 2;
    *((volatile uint32_t *)(base + IOAPIC_REGWIN)) = data & 0xFFFFFFFF;
    *((volatile uint32_t *)(base + IOAPIC_REGSEL)) = IOAPIC_REDTBL_BASE + index * 2 + 1;
    *((volatile uint32_t *)(base + IOAPIC_REGWIN)) = data >> 32;
}

uint64_t ioapic_get_entry(uint32_t base, uint32_t index)
{
    *((volatile uint32_t *)(base + IOAPIC_REGSEL)) = IOAPIC_REDTBL_BASE + index * 2;
    uint64_t lo = *((volatile uint32_t *)(base + IOAPIC_REGWIN));
    *((volatile uint32_t *)(base + IOAPIC_REGSEL)) = IOAPIC_REDTBL_BASE + index * 2 + 1;
    uint64_t hi = *((volatile uint32_t *)(base + IOAPIC_REGWIN));
    return (hi << 32) | lo;
}

void ioapic_install()
{
    if (!cpu.feature.edx.apic)
        return;
    for (uint32_t i = ioapic_addr; i < ioapic_addr + 0x1000; i += 0x1000)
    {
        page_alloc_frame(kernel_directory, i, i, 1, 1);
    }
    uint32_t x = ioapic_read(ioapic_addr, IOAPIC_VER);
    uint32_t irqs = ((x >> 16) & 0xff);
    for (uint8_t i = 0; i < irqs; i++)
        if (i == 0 || i > 15)
            ioapic_set_entry(ioapic_addr, acpi_get_irq(0x20 + i), 0x20 + i);
}