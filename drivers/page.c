#include <page.h>
#define INDEX_FROM_BIT(a) ((a) / 32)
#define OFFSET_FROM_BIT(a) ((a) % 32)

extern heap_t *kheap;

page_directory_t *kernel_directory = 0;
page_directory_t *current_directory = 0;

uint32_t *page_frames, page_frames_size;

bool page_check_frame(uint32_t address)
{
    uint32_t frame = address >> 12;
    uint32_t index = INDEX_FROM_BIT(frame);
    uint32_t offset = OFFSET_FROM_BIT(frame);
    return page_frames[index] & (1 << offset);
}

void page_set_frame(uint32_t address)
{
    uint32_t frame = address >> 12;
    uint32_t index = INDEX_FROM_BIT(frame);
    uint32_t offset = OFFSET_FROM_BIT(frame);
    page_frames[index] |= (1 << offset);
}

void page_clear_frame(uint32_t address)
{
    uint32_t frame = address >> 12;
    uint32_t index = INDEX_FROM_BIT(frame);
    uint32_t offset = OFFSET_FROM_BIT(frame);
    page_frames[index] &= ~(1 << offset);
}

uint32_t page_find_free_frame()
{
    if (!page_frames)
        return 0xffffffff;
    for (uint32_t i = 0; i < INDEX_FROM_BIT(page_frames_size); i++)
    {
        for (uint8_t j = 0; j < 32; j++)
            if (!(page_frames[i] & (1 << j)))
                return i * 32 + j;
    }
    return 0xffffffff;
}

void page_alloc_frame(page_directory_t *page_directory, uint32_t v_address, uint32_t p_address, bool is_kernel, bool is_writeable)
{
    if (!page_directory)
        return;
    page_t *page = get_page(v_address, 1, page_directory);
    uint32_t free_frame;
    if (p_address)
        free_frame = p_address;
    else
        free_frame = page_find_free_frame() << 12;
    page_set_frame(free_frame);
    page->present = 1;
    page->rw = (is_writeable) ? 1 : 0;
    page->user = (is_kernel) ? 0 : 1;
    page->frame = free_frame >> 12;
}

void page_free_frame(page_directory_t *page_directory, uint32_t v_address)
{
    if (!page_directory)
        return;
    page_t *page = get_page(v_address, 0, page_directory);
    if (!page)
        return;
    page->present = 0;
    page_clear_frame(page->frame << 12);
}

void page_install()
{
    uint32_t kheap_info[2] = {
        0,
    };

    page_frames_size = (uint32_t)-1 >> 12;
    page_frames = kmalloc(INDEX_FROM_BIT(page_frames_size) * sizeof(uint32_t));
    memset(page_frames, 0, INDEX_FROM_BIT(page_frames_size) * sizeof(uint32_t));
    kernel_directory = kmalloc_a(sizeof(page_directory_t), 0x1000);
    memset(kernel_directory, 0, sizeof(page_directory_t));
    kernel_directory->physicalAddr = KERNEL_V2P((uint32_t)&kernel_directory->tablesPhysical);
    current_directory = kernel_directory;

    for (uint32_t i = 0; i < KERNEL_V2P(placement_address); i += 0x1000)
    {
        page_alloc_frame(kernel_directory, KERNEL_P2V(i), i, 0, 0);
    }

    for (uint32_t i = placement_address; i < placement_address + 0x100000; i += 0x1000)
    {
        page_alloc_frame(kernel_directory, i, 0, 0, 0);
    }

    switch_page_directory(kernel_directory);
    enable_page();
    kheap = heap_create(placement_address, placement_address + 0x100000, 0xffffffff, 0, 0);
}

void switch_page_directory(page_directory_t *dir)
{
    current_directory = dir;
    asm volatile("mov %0, %%cr3" ::"r"(dir->physicalAddr));
}

void switch_page(void *page)
{
    asm volatile("mov %0, %%cr3" ::"r"((uint32_t)page));
}

void enable_page()
{
    uint32_t cr0, cr4;
    asm volatile("mov %%cr4, %0"
                 : "=r"(cr4));
    cr4 &= 0xffffffef;
    asm volatile("mov %0, %%cr4" ::"r"(cr4));
    asm volatile("mov %%cr0, %0"
                 : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" ::"r"(cr0));
}

page_t *get_page(uint32_t address, bool make, page_directory_t *dir)
{
    address = address >> 12;
    uint32_t table_idx = address / 1024, page_idx = address % 1024;
    if (dir->tablesPhysical[table_idx])
    {
        return &dir->tables[table_idx].pages[page_idx];
    }
    else if (make)
    {
        uint32_t tmp = KERNEL_V2P((uint32_t)&dir->tables[table_idx]);
        dir->tablesPhysical[table_idx] = tmp | 0x07;
        return &dir->tables[table_idx].pages[page_idx];
    }
    else
    {
        return (page_t *)NULL;
    }
}