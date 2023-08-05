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

uint8_t chr[0] = {
    '\0',
    '\0',
};

void loop()
{
    char key = NULL;
    while (true)
    {
        key = keyboard_get_key();
        if (key == '=')
            reboot();
        if (key)
            dos_print_char(key, 0x0f, 0x00);
    }
}

void kernel(multiboot_info_t *info)
{
    multiboot_module_t *mods = (multiboot_module_t *)KERNEL_P2V(info->mods_addr);
    gdt_install();
    idt_install();
    isr_install();
    irq_install();
    syscall_install();
    pit_install();
    keyboard_install();
    mouse_install();
    cpuid_install();
    fpu_install();
    bios32_install();
    page_install();
    acpi_install();
    vbe_install();
    vga_install();
    task_install();
    screen_install();
    extern uint32_t VIDEO_MEMORY;
    extern uint16_t vga_width, vga_height;
    if (screen_get_info())
    {
        VIDEO_MEMORY = (uint32_t)screen_get_info()->text_framebuffer;
        vga_width = screen_get_info()->current_video_mode->twidth;
        vga_height = screen_get_info()->current_video_mode->theight;
    }
    clear_screen();
    task_add(task_create("loop", loop, NULL));
}
