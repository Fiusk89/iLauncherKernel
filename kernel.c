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

void kernel_task()
{
    // sleep(1000);
    // return;
    // uint8_t byte = *((uint8_t *)0x00100000);
    // volatile uint32_t a = 0;
    // volatile uint32_t i = 8 / a;
    vbe_start();
}

uint8_t canvas_rgb_grayscale(uint32_t x, uint32_t y, uint32_t c);

void kernel(multiboot_info_t *info)
{
    multiboot_module_t *mods = KERNEL_P2V(info->mods_addr);
    if (info->mods_count > 0)
    {
        placement_address = KERNEL_P2V(mods[0].mod_end);
        ilrdfs_install(KERNEL_P2V(mods[0].mod_start));
        ilrdfs_list_nodes();
    }
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
    goto skip;
    vga_set_mode(vga_640x480x16);
    vga_set_palette(vga_palette);
    uint8_t backbuffer[640 * 480];
    uint32_t *image = tga_parse("system/app/ilframework/assets/pietro.tga");
    while (1)
    {
        memset(backbuffer, *backbuffer + 1, 640 * 480);
        for (uint32_t y = 0; y < image[1]; y++)
        {
            if (y < 480)
                for (uint32_t x = 0; x < image[0]; x++)
                {
                    uint32_t color = image[(y * image[0] + x) + 2];
                    if (x < 640)
                        backbuffer[y * 640 + x] = canvas_rgb_grayscale(x, y, color);
                }
        }
        /*for (uint8_t y = 0; y < 32; y++)
            if ((-mouse_ps2.delta_y) + y > -1 && (-mouse_ps2.delta_y) + y < 480)
                for (uint8_t x = 0; x < 16; x++)
                    if (mouse_ps2.delta_x + x > -1 && mouse_ps2.delta_x + x < 640)
                        backbuffer[(y + (-mouse_ps2.delta_y)) * 640 + (x + mouse_ps2.delta_x)] = 0xff;*/
        for (size_t plane = 0; plane < 4; plane++)
            vga_copy2framebuffer(backbuffer, plane);
        if (keyboard_get_key())
            reboot();
    }
skip:
    // pc_speaker_install();
    task_add(task_create("TASK2", kernel_task, 0));
}