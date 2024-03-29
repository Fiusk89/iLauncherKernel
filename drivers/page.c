#include <page.h>

extern heap_t *kheap;

page_directory_t *kernel_directory = 0;
page_directory_t *current_directory = 0;

uint64_t *page_frames, page_frames_size;

bool page_check_frame(uint32_t address)
{
    uint32_t frame = address >> 12;
    uint32_t index = BIT_INDEX(frame);
    uint32_t offset = BIT_OFFSET(frame);
    return page_frames[index] & (1 << offset);
}

void page_set_frame(uint32_t address)
{
    uint32_t frame = address >> 12;
    uint32_t index = BIT_INDEX(frame);
    uint32_t offset = BIT_OFFSET(frame);
    page_frames[index] |= (1 << offset);
}

void page_clear_frame(uint32_t address)
{
    uint32_t frame = address >> 12;
    uint32_t index = BIT_INDEX(frame);
    uint32_t offset = BIT_OFFSET(frame);
    page_frames[index] &= ~(1 << offset);
}

uint32_t page_get_free_frames()
{
    if (!page_frames)
        return 0xffffffff;
    uint32_t ret = 0;
    for (uint32_t i = 0; i < BIT_INDEX(page_frames_size); i++)
    {
        for (uint8_t j = 0; j < 64; j++)
            if (~page_frames[i] & (1 << j))
                ret++;
    }
    return ret;
}

uint32_t page_get_used_frames()
{
    if (!page_frames)
        return 0xffffffff;
    uint32_t ret = 0;
    for (uint32_t i = 0; i < BIT_INDEX(page_frames_size); i++)
    {
        for (uint8_t j = 0; j < 64; j++)
            if (page_frames[i] & (1 << j))
                ret++;
    }
    return ret;
}

uint32_t page_find_free_frame()
{
    if (!page_frames)
        return 0xffffffff;
    for (uint32_t i = 0; i < BIT_INDEX(page_frames_size); i++)
    {
        for (uint8_t j = 0; j < 64; j++)
            if (~page_frames[i] & (1 << j))
                return i * 64 + j;
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

    page_frames_size = (mm_addr + mm_length) >> 12;
    page_frames = (uint64_t *)kmalloc(BIT_INDEX(page_frames_size) * sizeof(uint64_t));
    memset(page_frames, 0, BIT_INDEX(page_frames_size) * sizeof(uint64_t));
    kernel_directory = (page_t *)kmalloc_a(sizeof(page_directory_t), 0x1000);
    memset(kernel_directory, 0, sizeof(page_directory_t));
    kernel_directory->physicalAddr = KERNEL_V2P((uint32_t)&kernel_directory->tablesPhysical);
    current_directory = kernel_directory;

    for (uint32_t i = 0; i < 0xfffff; i += 0x1000)
    {
        page_alloc_frame(kernel_directory, i, i, 0, 0);
    }

    for (uint32_t i = 0; i < KERNEL_V2P(placement_address); i += 0x1000)
    {
        page_alloc_frame(kernel_directory, KERNEL_P2V(i), i, 0, 0);
    }

    switch_page_directory(kernel_directory);
    page_enable();
}

void switch_page_directory(page_directory_t *dir)
{
    current_directory = dir;
    asm volatile("mov %0, %%cr3" ::"r"(dir->physicalAddr));
}

void page_enable()
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