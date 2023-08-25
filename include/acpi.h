#pragma once
#ifndef ACPI_H
#define ACPI_H
#include <kernel.h>
#define SCI_EN (1 << 0)
#define SLP_EN (1 << 13)
#define ACPI_IRQ_ENABLE (1 << 8) | (1 << 9)
#define ACPI_MAX_CPU 256 - 1

typedef struct RSDPDescriptor
{
    uint8_t Signature[8];
    uint8_t Checksum;
    uint8_t OEMID[6];
    uint8_t Revision;
    uint32_t RsdtAddress;
} __attribute__((packed)) acpi_rsdp_descriptor_t;

typedef struct ACPISDTHeader
{
    uint8_t Signature[4];
    uint32_t Length;
    uint8_t Revision;
    uint8_t Checksum;
    uint8_t OEMID[6];
    uint8_t OEMTableID[8];
    uint32_t OEMRevision;
    uint32_t CreatorID;
    uint32_t CreatorRevision;
} __attribute__((packed)) acpi_sdt_header_t;

typedef struct RSDT
{
    acpi_sdt_header_t header;
    uint32_t PointerToOtherSDT[16];
} __attribute__((packed)) acpi_rsdt_t;

typedef struct GenericAddressStructure
{
    uint8_t AddressSpace;
    uint8_t BitWidth;
    uint8_t BitOffset;
    uint8_t AccessSize;
    uint32_t Address;
} __attribute__((packed)) acpi_generic_address_structure_t;

typedef struct AcpiMadt
{
    acpi_sdt_header_t header;
    uint32_t localApicAddr;
    uint32_t flags;
} __attribute__((packed)) acpi_madt_t;

typedef struct AcpiMadtEntryHeader
{
    uint8_t type;
    uint8_t length;
} __attribute__((packed)) acpi_madt_entry_t;

typedef struct ApicLocalApic
{
    acpi_madt_entry_t header;
    uint8_t acpiProcessorId;
    uint8_t apicId;
    uint32_t flags;
} __attribute__((packed)) acpi_lapic_t;

typedef struct ApicIoApic
{
    acpi_madt_entry_t header;
    uint8_t ioApicId;
    uint8_t reserved;
    uint32_t ioApicAddress;
    uint32_t globalSystemInterruptBase;
} __attribute__((packed)) acpi_ioapic_t;

typedef struct ApicInterruptOverride
{
    acpi_madt_entry_t header;
    uint8_t bus;
    uint8_t source;
    uint32_t interrupt;
    uint16_t flags;
} __attribute__((packed)) acpi_apic_interrupt_override_t;

typedef struct FADT
{
    acpi_sdt_header_t header;
    uint32_t FirmwareCtrl;
    uint32_t Dsdt;
    uint8_t Reserved;
    uint8_t PreferredPowerManagementProfile;
    uint16_t SCI_Interrupt;
    uint32_t SMI_CommandPort;
    uint8_t AcpiEnable;
    uint8_t AcpiDisable;
    uint8_t S4BIOS_REQ;
    uint8_t PSTATE_Control;
    uint32_t PM1aEventBlock;
    uint32_t PM1bEventBlock;
    uint32_t PM1aControlBlock;
    uint32_t PM1bControlBlock;
    uint32_t PM2ControlBlock;
    uint32_t PMTimerBlock;
    uint32_t GPE0Block;
    uint32_t GPE1Block;
    uint8_t PM1EventLength;
    uint8_t PM1ControlLength;
    uint8_t PM2ControlLength;
    uint8_t PMTimerLength;
    uint8_t GPE0Length;
    uint8_t GPE1Length;
    uint8_t GPE1Base;
    uint8_t CStateControl;
    uint16_t WorstC2Latency;
    uint16_t WorstC3Latency;
    uint16_t FlushSize;
    uint16_t FlushStride;
    uint8_t DutyOffset;
    uint8_t DutyWidth;
    uint8_t DayAlarm;
    uint8_t MonthAlarm;
    uint8_t Century;
    uint16_t BootArchitectureFlags;
    uint8_t Reserved2;
    uint32_t Flags;
    acpi_generic_address_structure_t ResetReg;
    uint8_t ResetValue;
    uint8_t Reserved3[3];
} __attribute__((packed)) acpi_fadt_t;

extern uint8_t acpi_cpu_id[ACPI_MAX_CPU];
extern uint8_t acpi_cpu_cores;

void acpi_install(uint32_t start, uint32_t end);
int32_t acpi_get_irq(uint8_t irq);
int32_t acpi_get_irq_src(uint8_t irq);
bool acpi_shutdown();
void acpi_reboot();
#endif