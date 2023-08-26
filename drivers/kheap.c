#include <kheap.h>

heap_t *kheap;

extern uint32_t kernel_end;
uint32_t placement_address = (uint32_t)(&kernel_end);

uint32_t kmalloc_int(uint32_t size, uint16_t align, uint32_t *phys)
{
    if (kheap)
    {
        void *addr = heap_malloc(kheap, size, align);
        if (phys != 0)
        {
            page_t *page = get_page((uint32_t)addr, 0, kernel_directory);
            *phys = (page->frame << 12) + (uint32_t)addr & 0xfff;
        }
        return (uint32_t)addr;
    }
    else
    {
        if (align)
        {
            placement_address = KERNEL_ALIGN(placement_address, align);
        }
        if (phys)
        {
            *phys = KERNEL_V2P(placement_address);
        }
        uint32_t tmp = placement_address;
        placement_address += size;
        return tmp;
    }
}

void kheap_install()
{
    uint64_t aliged_placement_address = KERNEL_ALIGN(placement_address, 0x1000);
    for (uint64_t i = aliged_placement_address; i < aliged_placement_address + 0x4000; i += 0x1000)
    {
        page_alloc_frame(kernel_directory, i, KERNEL_V2P(i), 0, 0);
    }
    kheap = heap_create(aliged_placement_address, aliged_placement_address, aliged_placement_address + 0x4000, limit(aliged_placement_address + (page_get_free_frames() << 12), 0xffffffff), 0, 0);
}

uint32_t kmalloc_a(uint32_t size, uint16_t align)
{
    return kmalloc_int(size, align, 0);
}

uint32_t kmalloc_p(uint32_t size, uint32_t *phys)
{
    return kmalloc_int(size, 0, phys);
}

uint32_t kmalloc_ap(uint32_t size, uint16_t align, uint32_t *phys)
{
    return kmalloc_int(size, align, phys);
}

uint32_t kmalloc(uint32_t size)
{
    return kmalloc_int(size, 0, 0);
}

uint32_t krealloc(void *ptr, uint32_t size)
{
    return (uint32_t)heap_mrealloc(kheap, ptr, size);
}

uint32_t kclone(void *ptr)
{
    return (uint32_t)heap_mclone(kheap, ptr);
}

void kfree(void *ptr)
{
    heap_mfree(kheap, ptr);
}