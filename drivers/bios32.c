#include <bios32.h>

idtr_t real_idt_ptr;
idtr_t real_gdt_ptr;
extern bool is_idtr;

void (*rebased_bios32_helper)() = (void *)0x7c00;

void bios32_install()
{
    real_gdt_ptr.base = (uint32_t)&gdt;
    real_gdt_ptr.limit = sizeof(gdt) - 1;
    real_idt_ptr.base = 0;
    real_idt_ptr.limit = 0x3ff;
}

void bios32_service(uint8_t int_num, register16_t *in_reg, register16_t *out_reg)
{
    memcpy(&asm_gdt_entries, gdt, sizeof(gdt));
    real_gdt_ptr.base = (uint32_t)REBASE((&asm_gdt_entries));
    memcpy(&asm_gdt_ptr, &real_gdt_ptr, sizeof(real_gdt_ptr));
    memcpy(&asm_idt_ptr, &real_idt_ptr, sizeof(real_idt_ptr));
    memcpy(&asm_in_reg_ptr, in_reg, sizeof(register16_t));
    void *t = REBASE(&asm_in_reg_ptr);
    memcpy(&asm_intnum_ptr, &int_num, sizeof(uint8_t));
    memcpy(rebased_bios32_helper, bios32_helper, (uint32_t)bios32_helper_end - (uint32_t)bios32_helper);
    asm volatile("cli");
    rebased_bios32_helper();
    gdt_install();
    idt_install();
    asm volatile("sti");
    asm volatile("int $0x20");
    t = REBASE(&asm_out_reg_ptr);
    memcpy(out_reg, t, sizeof(register16_t));
}