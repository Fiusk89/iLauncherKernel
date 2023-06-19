#include <kernel.h>

extern void switch_usermode();

void poweroff()
{
    if (acpi_shutdown())
    {
        register16_t in_reg = {0}, out_reg = {0};
        in_reg.ax = 0x5301;
        bios32_service(0x15, &in_reg, &out_reg);
        in_reg.ax = 0x530E;
        in_reg.cx = 0x0102;
        bios32_service(0x15, &in_reg, &out_reg);
        in_reg.ax = 0x5307;
        in_reg.bx = 0x0001;
        in_reg.cx = 0x0003;
        bios32_service(0x15, &in_reg, &out_reg);
    }
}

void reboot()
{
    acpi_reboot();
    disable();
    uint8_t good = 0x02;
    while (good & 0x02)
        good = inb(0x64);
    outb(0x64, 0xFE);
    uint8_t temp;
    do
    {
        temp = inb(0x64);
        if (((temp) & (1 << (0))) != 0)
            inb(0x60);
    } while (((temp) & (1 << (1))) != 0);
    outb(0x64, 0xFE);
    while (1)
        farjump();
}

uint8_t canvas_rgb_grayscale(uint32_t x, uint32_t y, uint32_t c);

void kernel(multiboot_info_t *info)
{
    multiboot_module_t *mods = KERNEL_P2V(info->mods_addr);
    gdt_install();
    idt_install();
    isr_install();
    irq_install();
    syscall_install();
    pit_install();
    keyboard_install();
    mouse_install();
    vga_install();
    cpuid_install();
    fpu_install();
    bios32_install();
    page_install();
    acpi_install();
    task_install();
    screen_install();
    vbe_install();
    int8_t exceptions[] = {
        0,
        4,
        6,
        8,
        10,
        11,
        12,
        13,
        14,
        -1,
    };
    for (uint8_t i = 0; exceptions[i] != -1; i++)
        isr_add_handler(exceptions[i] ? (uint8_t)exceptions[i] : 0, task_fault);
    extern uint32_t VIDEO_MEMORY;
    extern uint16_t vga_width, vga_height;
    VIDEO_MEMORY = (uint32_t)screen_get_info()->text_framebuffer;
    vga_width = screen_get_info()->current_video_mode->twidth;
    vga_height = screen_get_info()->current_video_mode->theight;
    clear_screen();
    uint8_t chr[2];
    while (true)
    {
        chr[0] = keyboard_get_key();
        chr[1] = '\0';
        chr[0] == 'r' ? reboot() : 0;
        chr[0] == 's' ? poweroff() : 0;
        kprintf(chr);
    }
}