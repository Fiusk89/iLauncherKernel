#include <kernel.h>

const uint8_t bayer_16x16[16 * 16] = {
    0, 191, 48, 239, 12, 203, 60, 251, 3, 194, 51, 242, 15, 206, 63, 254,
    127, 64, 175, 112, 139, 76, 187, 124, 130, 67, 178, 115, 142, 79, 190, 127,
    32, 223, 16, 207, 44, 235, 28, 219, 35, 226, 19, 210, 47, 238, 31, 222,
    159, 96, 143, 80, 171, 108, 155, 92, 162, 99, 146, 83, 174, 111, 158, 95,
    8, 199, 56, 247, 4, 195, 52, 243, 11, 202, 59, 250, 7, 198, 55, 246,
    135, 72, 183, 120, 131, 68, 179, 116, 138, 75, 186, 123, 134, 71, 182, 119,
    40, 231, 24, 215, 36, 227, 20, 211, 43, 234, 27, 218, 39, 230, 23, 214,
    167, 104, 151, 88, 163, 100, 147, 84, 170, 107, 154, 91, 166, 103, 150, 87,
    2, 193, 50, 241, 14, 205, 62, 253, 1, 192, 49, 240, 13, 204, 61, 252,
    129, 66, 177, 114, 141, 78, 189, 126, 128, 65, 176, 113, 140, 77, 188, 125,
    34, 225, 18, 209, 46, 237, 30, 221, 33, 224, 17, 208, 45, 236, 29, 220,
    161, 98, 145, 82, 173, 110, 157, 94, 160, 97, 144, 81, 172, 109, 156, 93,
    10, 201, 58, 249, 6, 197, 54, 245, 9, 200, 57, 248, 5, 196, 53, 244,
    137, 74, 185, 122, 133, 70, 181, 118, 136, 73, 184, 121, 132, 69, 180, 117,
    42, 233, 26, 217, 38, 229, 22, 213, 41, 232, 25, 216, 37, 228, 21, 212,
    169, 106, 153, 90, 165, 102, 149, 86, 168, 105, 152, 89, 164, 101, 148, 85};

uint64_t mm_addr, mm_length;

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
    kprintf("USED MEMORY: %fMB;\nFREE MEMORY: %fMB;\n",
            (float)heap_get_used_size(kheap) / (float)MB,
            (float)heap_get_free_size(kheap) / (float)MB);
    while (true)
    {
        key = keyboard_get_key();
        if (key == '=')
            reboot();
        if (key == '+')
            poweroff();
        if (key == '\b')
        {
            remove_last_char();
            continue;
        }
        if (key)
            dos_print_char(key, 0x0f, 0x00);
    }
}

void kernel(multiboot_info_t *info)
{
    clear_screen();
    multiboot_memory_map_t *memory_map = (multiboot_memory_map_t *)KERNEL_P2V(info->mmap_addr);
    if (placement_address < KERNEL_P2V(info->mmap_addr + info->mmap_length))
        placement_address += KERNEL_P2V(info->mmap_addr + info->mmap_length) - placement_address;
    for (uint32_t i = 0; i < info->mmap_length / sizeof(multiboot_memory_map_t); i++)
    {
        if (memory_map[i].type == MULTIBOOT_MEMORY_AVAILABLE && memory_map[i].addr == 0x100000)
        {
            mm_addr = memory_map[i].addr;
            mm_length = memory_map[i].len;
            break;
        }
    }
    multiboot_module_t *modules = (multiboot_module_t *)KERNEL_P2V(info->mods_addr);
    if (placement_address < KERNEL_P2V(info->mods_addr + (sizeof(multiboot_module_t) * info->mods_count)))
        placement_address += KERNEL_P2V(info->mods_addr + (sizeof(multiboot_module_t) * info->mods_count));
    for (uint32_t i = 0; i < info->mods_count; i++)
    {
        if (placement_address < KERNEL_P2V(modules[i].mod_end))
            placement_address += KERNEL_P2V(modules[i].mod_end) - placement_address;
    }
    gdt_install();
    idt_install();
    isr_install();
    irq_install();
    pit_install();
    page_install();
    kheap_install();
    devfs_install();
    for (uint32_t i = 0; i < info->mods_count; i++)
        ramfs_add((void *)KERNEL_P2V(modules[i].mod_start), (void *)KERNEL_P2V(modules[i].mod_end));
    fs_root = ilfs_create("ramdisk0");
    fs_mount(fs_root, "dev", fs_dev);
    uhci_install();
    syscall_install();
    keyboard_install();
    mouse_install();
    cpuid_install();
    fpu_install();
    bios32_install();
    acpi_install(0xe0000, 0xfffff);
    vga_install();
    vbe_install();
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
