#include <smp.h>

void smp_install()
{
    if (!lapic_addr)
        return;
    uint32_t local_id = lapic_id();
    for (uint32_t i = 0; i < acpi_cpu_cores; i++)
    {
        uint32_t apic_id = acpi_cpu_id[i];
        if (apic_id != local_id)
            lapic_cpu_enable(apic_id);
    }
    for (uint32_t i = 0; i < acpi_cpu_cores; i++)
    {
        uint32_t apic_id = acpi_cpu_id[i];
        if (apic_id != local_id)
            lapic_cpu_start(apic_id, 0x08);
    }
}