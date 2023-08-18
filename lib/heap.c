#include <heap.h>

heap_t *heap_create(uint32_t v_start, uint32_t p_start, uint64_t end, uint64_t max, uint8_t is_kernel, uint8_t is_writable)
{
    if (v_start + sizeof(heap_t) + sizeof(heap_node_t) > end)
        return (void *)NULL;
    if (v_start + sizeof(heap_t) + sizeof(heap_node_t) > max)
        return (void *)NULL;
    if (end > max)
        return (void *)NULL;
    heap_t *heap = (heap_t *)p_start;
    v_start += sizeof(heap_t), p_start += sizeof(heap_t);
    heap->start = v_start;
    heap->end = end;
    heap->max = max;
    heap->is_kernel = is_kernel;
    heap->is_writable = is_writable;
    heap_node_t *heap_node = (heap_node_t *)p_start;
    heap_node->signature = HEAP_SIGNATURE;
    heap_node->is_free = true;
    heap_node->size = heap->max - heap->start;
    heap_node->align = NULL;
    heap_node->prev = (heap_node_t *)NULL;
    heap_node->next = (heap_node_t *)NULL;
    return heap;
}

void heap_show_all_nodes(heap_t *heap)
{
    heap_node_t *tmp = (heap_node_t *)heap->start;
    while (tmp)
    {
        kprintf("heap_node : {\n");
        kprintf("\tuint64_t signature = %x;\n", (uint32_t)tmp->signature);
        kprintf("\tuint8_t is_free = %s;\n", tmp->is_free ? "true" : "false");
        kprintf("\tuint64_t size = %u;\n", (uint32_t)tmp->size);
        kprintf("\tuint16_t align = %u;\n", (uint32_t)tmp->align);
        kprintf("};\n");
        tmp = tmp->next;
    }
}

static void heap_expand(heap_t *heap, uint64_t new_size)
{
    new_size = KERNEL_ALIGN(new_size, 0x1000);
    if (!heap || !new_size)
        return;
    if (new_size < heap->end - heap->start || new_size > heap->max - heap->start)
        return;
    for (uint64_t i = heap->end - heap->start; i < new_size; i += 0x1000)
    {
        page_alloc_frame(kernel_directory, heap->start + i, NULL, heap->is_kernel, heap->is_writable);
    }
    heap->end = heap->start + new_size;
}

static void heap_contract(heap_t *heap, uint64_t new_size)
{
    new_size = KERNEL_ALIGN(new_size, 0x1000);
    if (!heap || !new_size)
        return;
    if (new_size > heap->end - heap->start)
        return;
    if (new_size < 0x1000)
        new_size = 0x1000;
    for (uint64_t i = new_size; i < heap->end - heap->start; i += 0x1000)
    {
        page_free_frame(kernel_directory, heap->start + i);
    }
    heap->end = heap->start + new_size;
}

static void heap_expand_free_nodes(heap_t *heap)
{
    if (!heap)
        return;
    while (true)
    {
        heap_node_t *heap_node = (heap_node_t *)heap->start;
        heap_node_t *heap_node_end = (heap_node_t *)heap->start;
        while (heap_node)
        {
            if (heap_node->is_free)
                if (heap_node->next)
                    if (heap_node->next->is_free)
                        break;
            heap_node = heap_node->next;
        }
        if (!heap_node)
            return;
        uint64_t new_size = heap_node->size;
        heap_node_end = heap_node->next;
        while (heap_node_end)
        {
            if (!heap_node_end->is_free)
                break;
            new_size += sizeof(heap_node_t) + heap_node_end->size;
            heap_node_end = heap_node_end->next;
        }
        if (heap_node_end)
            heap_node->size = new_size - sizeof(heap_node_t);
        else
            heap_node->size = new_size;
        heap_node->align = NULL;
        heap_node->next = heap_node_end;
        if (heap_node_end)
            heap_node_end->prev = heap_node;
    }
}

uint64_t heap_get_free_size(heap_t *heap)
{
    if (!heap)
        return NULL;
    heap_node_t *heap_node = (heap_node_t *)heap->start;
    uint64_t ret = 0;
    while (heap_node)
    {
        if (heap_node->is_free)
            ret += sizeof(heap_node_t) + heap_node->size;
        heap_node = heap_node->next;
    }
    return ret;
}

uint64_t heap_get_used_size(heap_t *heap)
{
    if (!heap)
        return NULL;
    heap_node_t *heap_node = (heap_node_t *)heap->start;
    uint64_t ret = 0;
    while (heap_node)
    {
        if (!heap_node->is_free)
            ret += sizeof(heap_node_t) + heap_node->size;
        heap_node = heap_node->next;
    }
    return ret;
}

uint64_t heap_get_size(heap_t *heap)
{
    if (!heap)
        return NULL;
    return heap->end - heap->start;
}

void *heap_malloc(heap_t *heap, uint64_t size, uint16_t align)
{
    if (!heap || !size)
        return (void *)NULL;
    if (align)
        size = KERNEL_ALIGN(size + align, align);
    size += sizeof(uint64_t);
    uint64_t full_size = sizeof(heap_node_t) * 3 + size;
    heap_node_t *heap_node = (heap_node_t *)heap->start;
    while (heap_node->next)
    {
        if (heap_node->size >= size && heap_node->is_free)
            break;
        heap_node = heap_node->next;
    }
    if (heap_node->size == size)
    {
        uint64_t address = NULL;
        heap_node->is_free = false;
        heap_node->align = align;
        if (align)
            address = KERNEL_ALIGN((uint64_t)heap_node + sizeof(heap_node_t) + sizeof(uint64_t), align);
        else
            address = (uint64_t)heap_node + sizeof(heap_node_t) + sizeof(uint64_t);
        (*(uint64_t *)(address - sizeof(uint64_t))) = (uint64_t)heap_node;
        return (void *)address;
    }
    else if (heap_node->size > size)
    {
        uint64_t address = NULL;
        heap_node_t *new_heap_node = (heap_node_t *)((uint64_t)heap_node + (full_size - sizeof(heap_node_t)));
        if ((uint64_t)new_heap_node + sizeof(heap_node_t) > heap->end)
        {
            heap_expand(heap, ((uint64_t)new_heap_node + sizeof(heap_node_t)) - heap->start);
            if ((uint64_t)new_heap_node + sizeof(heap_node_t) > heap->end)
                return (void *)NULL;
        }
        memset(new_heap_node, 0, sizeof(heap_node_t));
        new_heap_node->signature = HEAP_SIGNATURE;
        new_heap_node->is_free = true;
        new_heap_node->size = heap_node->size - size;
        new_heap_node->prev = heap_node;
        new_heap_node->next = heap_node->next;
        heap_node->signature = HEAP_SIGNATURE;
        heap_node->is_free = false;
        heap_node->size = size;
        heap_node->align = align;
        heap_node->next = new_heap_node;
        if (align)
            address = KERNEL_ALIGN((uint64_t)heap_node + sizeof(heap_node_t) + sizeof(uint64_t), align);
        else
            address = (uint64_t)heap_node + sizeof(heap_node_t) + sizeof(uint64_t);
        (*(uint64_t *)(address - sizeof(uint64_t))) = (uint64_t)heap_node;
        return (void *)address;
    }
    return (void *)NULL;
}

void *heap_mexpand(heap_t *heap, void *ptr, int64_t size)
{
    if (!heap || !ptr)
        return (void *)NULL;
    heap_node_t *node = (heap_node_t *)(*(uint64_t *)((uint64_t)ptr - sizeof(uint64_t)));
    if (node->signature != HEAP_SIGNATURE)
        return (void *)NULL;
    if (!size)
    {
        heap_mfree(heap, ptr);
        return (void *)NULL;
    }
    else
    {
        void *new_ptr = heap_malloc(heap, node->size + size, node->align);
        if (!new_ptr)
            return (void *)NULL;
        memcpy(new_ptr, ptr, node->size);
        heap_mfree(heap, ptr);
        return new_ptr;
    }
}

void *heap_mrealloc(heap_t *heap, void *ptr, uint64_t size)
{
    if (!heap || !ptr)
        return (void *)NULL;
    heap_node_t *node = (heap_node_t *)(*(uint64_t *)((uint64_t)ptr - sizeof(uint64_t)));
    if (node->signature != HEAP_SIGNATURE)
        return (void *)NULL;
    if (!size)
    {
        heap_mfree(heap, ptr);
        return (void *)NULL;
    }
    else
    {
        void *new_ptr = heap_malloc(heap, size, node->align);
        if (!new_ptr)
            return (void *)NULL;
        memcpy(new_ptr, ptr, node->size);
        heap_mfree(heap, ptr);
        return new_ptr;
    }
}

void *heap_mclone(heap_t *heap, void *ptr)
{
    if (!heap || !ptr)
        return (void *)NULL;
    heap_node_t *node = (heap_node_t *)(*(uint64_t *)((uint64_t)ptr - sizeof(uint64_t)));
    if (node->signature != HEAP_SIGNATURE)
        return (void *)NULL;
    void *new_ptr = heap_malloc(heap, node->size, node->align);
    if (!new_ptr)
        return (void *)NULL;
    memcpy(new_ptr, ptr, node->size);
    return new_ptr;
}

void heap_mfree(heap_t *heap, void *ptr)
{
    if (!heap || !ptr)
        return;
    heap_node_t *node = (heap_node_t *)(*(uint64_t *)((uint64_t)ptr - sizeof(uint64_t)));
    if (node->signature != HEAP_SIGNATURE)
        return;
    node->is_free = true;
    heap_expand_free_nodes(heap);
    if (!node->next)
        heap_contract(heap, ((uint64_t)node + sizeof(heap_node_t)) - heap->start);
}