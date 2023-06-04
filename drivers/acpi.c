#include <acpi.h>

acpi_madt_t *madt;
acpi_rsdt_t *rsdt;
acpi_fadt_t *facp;
acpi_sdt_header_t *dsdt;
acpi_sdt_header_t *ssdt;
uint8_t acpi_cpu_id[ACPI_MAX_CPU];
uint8_t acpi_cpu_cores = 0;

void AcpiInt_Handler(register_t *r)
{
    reboot();
}

bool acpi_check_apic()
{
    if (cpu.feature.edx.apic && madt)
        return true;
    else
        return false;
}

void acpi_allocate_region(void *p_addr, void *length)
{
    for (uint32_t i = (uint32_t)p_addr; i < (uint32_t)p_addr + (uint32_t)length; i += 0x1000)
    {
        page_alloc_frame(kernel_directory, i, i, 1, 1);
    }
}

void acpi_enable()
{
    if (!facp)
        return;
    outb(facp->SMI_CommandPort, facp->AcpiEnable);
    while (!(facp->PM1aControlBlock && SCI_EN))
        ;
    if (facp->PM1bControlBlock)
    {
        outb(facp->SMI_CommandPort, facp->AcpiEnable);
        while (!(facp->PM1bControlBlock && SCI_EN))
            ;
    }
    outw(facp->PM1aEventBlock, ACPI_IRQ_ENABLE);
    if (facp->SCI_Interrupt >= 32)
        irq_add_handler(facp->SCI_Interrupt - 32, AcpiInt_Handler);
    else
        irq_add_handler(facp->SCI_Interrupt, AcpiInt_Handler);
}

int32_t acpi_get_irq(uint8_t irq)
{
    uint32_t start = (uint32_t)madt + sizeof(acpi_madt_t);
    uint32_t end = (uint32_t)madt + madt->header.Length;
    while (start < end)
    {
        acpi_madt_entry_t *header = start;
        if (header->type == 2)
        {
            acpi_apic_interrupt_override_t *s = start;
            if (s->source == irq)
                return s->interrupt;
        }
        start += header->length;
    }
    return -1;
}

uint8_t acpi_detect_cpu()
{
    uint32_t start = (uint32_t)madt + sizeof(acpi_madt_t);
    uint32_t end = (uint32_t)madt + madt->header.Length;
    while (start < end)
    {
        acpi_madt_entry_t *header = start;
        if (header->type == 0)
        {
            acpi_lapic_t *s = start;
            acpi_cpu_id[acpi_cpu_cores] = s->apicId;
            acpi_cpu_cores++;
        }
        start += header->length;
    }
}

int32_t acpi_get_irq_src(uint8_t irq)
{
    uint32_t start = (uint32_t)madt + sizeof(acpi_madt_t);
    uint32_t end = (uint32_t)madt + madt->header.Length;
    while (start < end)
    {
        acpi_madt_entry_t *header = start;
        if (header->type == 2)
        {
            acpi_apic_interrupt_override_t *s = start;
            if (s->source == irq)
                return s->source;
        }
        start += header->length;
    }
    return -1;
}

void acpi_install()
{
    acpi_rsdp_descriptor_t *rsdp;
    acpi_sdt_header_t *sdt;
    acpi_allocate_region(0, 0xfffff);
    for (uint32_t ptr = 0xe0000; ptr < 0xfffff; ptr += 16)
    {
        if (!strncmp((char *)ptr, "RSD PTR ", 8))
        {
            rsdp = (acpi_rsdp_descriptor_t *)ptr;
            rsdt = (acpi_rsdt_t *)rsdp->RsdtAddress;
            acpi_allocate_region(rsdp->RsdtAddress, 0x1000);
            acpi_allocate_region(rsdp->RsdtAddress, rsdt->header.Length);
            uint32_t i_max = (rsdt->header.Length - sizeof(acpi_sdt_header_t)) / 4;
            for (uint32_t i = 0; i < i_max; i++)
            {
                sdt = rsdt->PointerToOtherSDT[i];
                acpi_allocate_region(sdt, 0x1000);
                acpi_allocate_region(sdt, sdt->Length);
                if (!strncmp(sdt->Signature, "SSDT", 4))
                {
                    ssdt = sdt;
                }
                if (!strncmp(sdt->Signature, "FACP", 4))
                {
                    facp = sdt;
                    dsdt = facp->Dsdt;
                    acpi_allocate_region(facp->Dsdt, 0x1000);
                    acpi_allocate_region(facp->Dsdt, dsdt->Length);
                    if (strncmp(dsdt->Signature, "DSDT", 4))
                        dsdt = NULL;
                }
                if (!strncmp(sdt->Signature, "APIC", 4))
                {
                    madt = sdt;
                    lapic_addr = madt->localApicAddr;
                    uint32_t start = (uint32_t)madt + sizeof(acpi_madt_t);
                    uint32_t end = (uint32_t)madt + madt->header.Length;
                    acpi_allocate_region(start, end - start);
                    while (start < end)
                    {
                        acpi_madt_entry_t *header = start;
                        if (header->type == 1)
                        {
                            acpi_ioapic_t *s = header;
                            ioapic_addr = s->ioApicAddress;
                        }
                        start += header->length;
                    }
                }
            }
            acpi_detect_cpu();
            acpi_enable();
            return;
        }
    }
    kprintf("Acpi error\n");
}

bool acpi_sdt_s5_send_cmd(acpi_sdt_header_t *sdt, acpi_fadt_t *fadt, uint16_t cmd)
{
    uint32_t start = (uint32_t)sdt + sizeof(acpi_sdt_header_t), end = (uint32_t)sdt + sdt->Length;
    for (uint32_t i = start; i < end; i++)
    {
        uint8_t *S5_ACPI = (uint8_t *)i;
        if (!strncmp(S5_ACPI, "_S5_", 4))
        {
            if (S5_ACPI[7] == 0x0A)
                S5_ACPI++;
            outw(fadt->PM1aControlBlock, S5_ACPI[7] << 10 | cmd);
            if (fadt->PM1bControlBlock)
                outw(fadt->PM1bControlBlock, S5_ACPI[9] << 10 | cmd);
            return true;
        }
    }
    return false;
}

bool acpi_shutdown()
{
    if (!facp)
        return false;
    if (!ssdt)
    {
        if (!acpi_sdt_s5_send_cmd(dsdt, facp, SLP_EN))
            return acpi_sdt_s5_send_cmd(dsdt, facp, SLP_EN);
        else
            return true;
    }
    if (ssdt->Length > dsdt->Length)
    {
        if (acpi_sdt_s5_send_cmd(dsdt, facp, SLP_EN))
            return true;
        else if (!acpi_sdt_s5_send_cmd(ssdt, facp, SLP_EN))
            if (acpi_sdt_s5_send_cmd(dsdt, facp, SLP_EN))
                return true;
            else
                return acpi_sdt_s5_send_cmd(ssdt, facp, SLP_EN);
    }
    else
    {
        if (acpi_sdt_s5_send_cmd(ssdt, facp, SLP_EN))
            return true;
        else if (!acpi_sdt_s5_send_cmd(dsdt, facp, SLP_EN))
            if (acpi_sdt_s5_send_cmd(ssdt, facp, SLP_EN))
                return true;
            else
                return acpi_sdt_s5_send_cmd(dsdt, facp, SLP_EN);
    }
    return true;
}

void acpi_reboot()
{
    outb(facp->ResetReg.Address, facp->ResetValue);
}