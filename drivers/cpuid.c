#include <cpuid.h>

cpuid_t cpu = {0};

void cpuid(uint32_t reg, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx)
{
    asm volatile("cpuid"
                 : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                 : "0"(reg));
}

void cpuid_install()
{
    uint32_t eax, ebx, ecx, edx;
    uint32_t tmpreg;
    char vendor[14];
    memset(vendor, 0, 14);
    cpuid(0, &tmpreg, (uint32_t *)(vendor + 0), (uint32_t *)(vendor + 8), (uint32_t *)(vendor + 4));

    for (uint8_t i = 0; i < 14; i++)
        cpu.vendor[i] = vendor[i];

    if (tmpreg >= 0x01)
    {
        cpuid(0x01, &eax, &ebx, &ecx, &edx);
        if (edx & EDX_ACPI)
            cpu.feature.edx.acpi = true;
        else
            cpu.feature.edx.acpi = false;
        if (edx & EDX_FPU)
            cpu.feature.edx.fpu = true;
        else
            cpu.feature.edx.fpu = false;
        if (edx & EDX_PSE)
            cpu.feature.edx.pse = true;
        else
            cpu.feature.edx.pse = false;
        if (edx & EDX_PAE)
            cpu.feature.edx.pae = true;
        else
            cpu.feature.edx.pae = false;
        if (edx & EDX_APIC)
            cpu.feature.edx.apic = true;
        else
            cpu.feature.edx.apic = false;
        if (edx & EDX_MTRR)
            cpu.feature.edx.mtrr = true;
        else
            cpu.feature.edx.mtrr = false;
        if (edx & EDX_TSC)
            cpu.feature.edx.tsc = true;
        else
            cpu.feature.edx.tsc = false;
        if (edx & EDX_MSR)
            cpu.feature.edx.msr = true;
        else
            cpu.feature.edx.msr = false;
        if (edx & EDX_SSE)
            cpu.feature.edx.sse = true;
        else
            cpu.feature.edx.sse = false;
        if (edx & EDX_SSE2)
            cpu.feature.edx.sse2 = true;
        else
            cpu.feature.edx.sse2 = false;
        if (edx & EDX_MMX)
            cpu.feature.edx.mmx = true;
        else
            cpu.feature.edx.mmx = false;
        if (ecx & ECX_SSE3)
            cpu.feature.ecx.sse3 = true;
        else
            cpu.feature.ecx.sse3 = false;
        if (ecx & ECX_SSSE3)
            cpu.feature.ecx.ssse3 = true;
        else
            cpu.feature.ecx.ssse3 = false;
        if (ecx & ECX_SSE41)
            cpu.feature.ecx.sse41 = true;
        else
            cpu.feature.ecx.sse41 = false;
        if (ecx & ECX_SSE42)
            cpu.feature.ecx.sse42 = true;
        else
            cpu.feature.ecx.sse42 = false;
        if (ecx & ECX_AVX)
            cpu.feature.ecx.avx = true;
        else
            cpu.feature.ecx.avx = false;
        if (ecx & ECX_F16C)
            cpu.feature.ecx.f16c = true;
        else
            cpu.feature.ecx.f16c = false;
        if (ecx & ECX_RDRAND)
            cpu.feature.ecx.rdrand = true;
        else
            cpu.feature.ecx.rdrand = false;
    }
    cpuid(0x80000000, &tmpreg, &ebx, &ecx, &edx);
    if (tmpreg >= 0x80000001)
    {
        cpuid(0x80000001, &eax, &ebx, &ecx, &edx);

        if (edx & EDX_64_BIT)
            cpu.feature.edx.x86_64 = true;
        else
            cpu.feature.edx.x86_64 = false;
    }

    if (tmpreg >= 0x80000004)
    {
        char name[48];
        memset(name, 0, 48);
        cpuid(0x80000002, (uint32_t *)(name + 0), (uint32_t *)(name + 4), (uint32_t *)(name + 8), (uint32_t *)(name + 12));
        cpuid(0x80000003, (uint32_t *)(name + 16), (uint32_t *)(name + 20), (uint32_t *)(name + 24), (uint32_t *)(name + 28));
        cpuid(0x80000004, (uint32_t *)(name + 32), (uint32_t *)(name + 36), (uint32_t *)(name + 40), (uint32_t *)(name + 44));
        for (uint32_t i = 0; i < 48; i++)
            cpu.name[i] = name[i];
    }
}