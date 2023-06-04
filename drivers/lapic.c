#include <apic.h>

uint32_t lapic_addr;

#define LAPIC_REGISTER_ID 0x20
#define LAPIC_REGISTER_VERSION 0x30
#define LAPIC_REGISTER_TASK_PRIO 0x80
#define LAPIC_REGISTER_ARBITRATION_PRIO 0x90
#define LAPIC_REGISTER_EOI 0xB0
#define LAPIC_REGISTER_LOGICAL_DEST 0xD0
#define LAPIC_REGISTER_DEST_FORMAT 0xE0
#define LAPIC_REGISTER_SPURIOUS_IVT 0xF0
#define LAPIC_REGISTER_ERROR_STATUS 0x280
#define LAPIC_REGISTER_LVT_CMCI 0x2F0
#define LAPIC_REGISTER_INT_COMMAND_LOW 0x300
#define LAPIC_REGISTER_INT_COMMAND_HIGH 0x310
#define LAPIC_REGISTER_LVT_TIMER 0x320
#define LAPIC_REGISTER_LVT_THERMAL_SENSOR 0x320
#define LAPIC_REGISTER_LVT_PERFMON 0x340
#define LAPIC_REGISTER_LVT_LINT0 0x350
#define LAPIC_REGISTER_LVT_LINT1 0x360
#define LAPIC_REGISTER_LVT_ERROR 0x370
#define LAPIC_REGISTER_TIMER_INITCNT 0x380
#define LAPIC_REGISTER_TIMER_CURRCNT 0x390
#define LAPIC_REGISTER_TIMER_DIV 0x3E0
#define LAPIC_SPURIOUS_IVT_SOFTWARE_ENABLE 0x100
#define LAPIC_LVT_DELIVERY_MODE_FIXED (0 << 8)
#define LAPIC_LVT_DELIVERY_MODE_SMI (2 << 8)
#define LAPIC_LVT_DELIVERY_MODE_NMI (4 << 8)
#define LAPIC_LVT_DELIVERY_MODE_INIT (5 << 8)
#define LAPIC_LVT_DELIVERY_MODE_EXTINT (7 << 8)
#define LAPIC_LVT_GET_DELIVERY_STATUS(i) ((i >> 12) & 1)
#define LAPIC_LVT_INT_POLARITY_HIGH (1 << 13)
#define LAPIC_LVT_INT_POLARITY_LOW (0 << 13)
#define LAPIC_LVT_GET_REMOTE_IRR(i) ((i << 14) & 1)
#define LAPIC_LVT_TRIGGER_MODE_EDGE (0 << 15)
#define LAPIC_LVT_TRIGGER_MODE_LEVEL (1 << 15)
#define LAPIC_LVT_INT_UNMASKED (0 << 16)
#define LAPIC_LVT_INT_MASKED (1 << 16)
#define LAPIC_LVT_TIMER_MODE_ONESHOT (0 << 17)
#define LAPIC_LVT_TIMER_MODE_PERIODIC (1 << 17)
#define LAPIC_LVT_TIMER_MODE_TSC_DEADLINE (2 << 17)
#define LAPIC_ICR_INTVEC_MAKE (0xFF)
#define LAPIC_ICR_DELMOD_FIXED (0 << 8)
#define LAPIC_ICR_DELMOD_LOWEST (1 << 8)
#define LAPIC_ICR_DELMOD_SMI (2 << 8)
#define LAPIC_ICR_DELMOD_NMI (4 << 8)
#define LAPIC_ICR_DELMOD_INIT (5 << 8)
#define LAPIC_ICR_DELMOD_SIPI (6 << 8)
#define LAPIC_ICR_DESTMOD_PHYSICAL (0 << 11)
#define LAPIC_ICR_DESTMOD_LOGICAL (1 << 11)
#define LAPIC_ICR_DELIVS_IDLE (0 << 12)
#define LAPIC_ICR_DELIVS_SEND_PENDING (1 << 12)
#define LAPIC_ICR_LEVEL_DEASSERT (0 << 14)
#define LAPIC_ICR_LEVEL_ASSERT (1 << 14)
#define LAPIC_ICR_TRIGGERMOD_EDGE (0 << 15)
#define LAPIC_ICR_TRIGGERMOD_LEVEL (1 << 15)
#define LAPIC_ICR_DEST_SHORTHAND_NONE (0 << 18)
#define LAPIC_ICR_DEST_SHORTHAND_SELF (1 << 18)
#define LAPIC_ICR_DEST_SHORTHAND_ALL_WITH_SELF (2 << 18)
#define LAPIC_ICR_DEST_SHORTHAND_ALL_NOT_SELF (3 << 18)
#define LAPIC_ICR_DESTINATION_MAKE(i) (((uint64_t)i & 0xFF) << 56)

static uint32_t lapic_read(uint32_t reg)
{
	return mm_inl(lapic_addr + reg);
}

static void lapic_write(uint32_t reg, uint32_t data)
{
	mm_outl(lapic_addr + reg, data);
}

uint32_t lapic_id()
{
	return (lapic_read(LAPIC_REGISTER_ID) >> 24) & 0xFF;
}

void lapic_enable(uint32_t apic)
{
	uint32_t edx = 0;
	uint32_t eax = (apic & 0xfffff0000) | 0x800;
	cpu_write_msr(0x1B, eax, edx);
}

void lapic_install()
{
	if (!cpu.feature.edx.apic)
		return;
	for (uint32_t i = lapic_addr; i < lapic_addr + 0x1000; i += 0x1000)
    {
        page_alloc_frame(kernel_directory, i, i, 1, 1);
    }
	lapic_write(LAPIC_REGISTER_TASK_PRIO, 0);
	lapic_write(LAPIC_REGISTER_DEST_FORMAT, 0xffffffff);
	lapic_write(LAPIC_REGISTER_LOGICAL_DEST, 0x01000000);
	lapic_write(LAPIC_REGISTER_SPURIOUS_IVT, 0xFF | LAPIC_SPURIOUS_IVT_SOFTWARE_ENABLE);
}

void lapic_cpu_enable(uint32_t apic_id)
{
	lapic_write(LAPIC_REGISTER_INT_COMMAND_HIGH, apic_id << 24);
	lapic_write(LAPIC_REGISTER_INT_COMMAND_LOW, LAPIC_LVT_DELIVERY_MODE_INIT | 0x00 | 0x40000 | 0x00 | 0x00);
	while (lapic_read(LAPIC_REGISTER_INT_COMMAND_LOW) & 0x1000)
		;
}

void lapic_cpu_start(uint32_t apic_id, uint32_t vector)
{
	lapic_write(LAPIC_REGISTER_INT_COMMAND_HIGH, apic_id << 24);
	lapic_write(LAPIC_REGISTER_INT_COMMAND_LOW, vector | 0x600 | 0x00 | 0x40000 | 0x00 | 0x00);
	while (lapic_read(LAPIC_REGISTER_INT_COMMAND_LOW) & 0x1000)
		;
}

void lapic_end()
{
	lapic_write(LAPIC_REGISTER_EOI, 0x00);
}