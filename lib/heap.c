#include <heap.h>

heap_t *heap_create(uint64_t start, uint64_t end, uint64_t max, uint8_t is_kernel, uint8_t is_writable)
{
    heap_t *heap = (heap_t *)start;
    start += sizeof(heap_t);
    heap->start = start;
    heap->end = end;
    heap->max = max;
    heap_node_t *heap_node = (heap_node_t *)heap->start;
    heap_node->signature = HEAP_SIGNATURE;
    heap_node->is_free = true;
    heap_node->size = heap->max - heap->start;
    heap_node->align = NULL;
    heap_node->prev = (heap_node_t *)NULL;
    heap_node->next = (heap_node_t *)NULL;
    return heap;
}

static void heap_expand(heap_t *heap, uint64_t new_size)
{
    if (!heap || !new_size)
        return;
    if (new_size < heap->end - heap->start)
        return;
    if (new_size > heap->max - heap->start)
        return;
    for (uint32_t i = heap->end - heap->start; i < new_size; i += 0x1000)
    {
        page_alloc_frame(kernel_directory, heap->start + i, NULL, heap->is_kernel, heap->is_writable);
    }
    heap->end = heap->start + new_size;
}

static void heap_contract(heap_t *heap, uint64_t new_size)
{
    if (!heap || !new_size)
        return;
    if (new_size > heap->end - heap->start)
        return;
    if (new_size < sizeof(heap_node_t))
        new_size = sizeof(heap_node_t);
    for (uint32_t i = new_size; i < heap->end - heap->start; i += 0x1000)
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
        while (heap_node->next)
        {
            if (heap_node->next->signature != HEAP_SIGNATURE)
                break;
            if (heap_node->is_free)
                if (heap_node->next)
                    if (heap_node->next->is_free && heap_node->next->next)
                        break;
            heap_node = heap_node->next;
        }
        heap_node_end = heap_node->next;
        if (!heap_node_end)
            return;
        while (heap_node_end->next)
        {
            if (heap_node_end->next->signature != HEAP_SIGNATURE)
                break;
            if (!heap_node_end->is_free)
                break;
            heap_node->size += sizeof(heap_node_t) + heap_node_end->size;
            heap_node_end = heap_node_end->next;
        }
        if (heap_node_end)
        {
            heap_node->next = heap_node_end;
            heap_node_end->prev = heap_node;
        }
    }
}

uint64_t heap_get_used_size(heap_t *heap)
{
    if (!heap)
        return NULL;
    uint64_t ret = 0;
    heap_node_t *heap_node = (heap_node_t *)heap->start;
    while (heap_node->next)
    {
        if (heap_node->next->signature != HEAP_SIGNATURE)
            break;
        if (!heap_node->is_free)
            ret += sizeof(heap_node_t) + heap_node->size;
        heap_node = heap_node->next;
    }
    return ret > 0 ? ret : sizeof(heap_node_t);
}

void *heap_malloc(heap_t *heap, uint64_t size, uint16_t align)
{
    if (!heap || !size)
        return (void *)NULL;
    if (align)
        size = KERNEL_ALIGN(size + align, align);
    size += sizeof(uint64_t);
    if (heap->end - heap->start < heap_get_used_size(heap) + sizeof(heap_node_t) + size)
    {
        heap_expand(heap, heap_get_used_size(heap) + sizeof(heap_node_t) + size);
        if (heap->end - heap->start < heap_get_used_size(heap) + sizeof(heap_node_t) + size)
            return (void *)NULL;
    }
    heap_expand_free_nodes(heap);
    heap_node_t *heap_node = heap->start;
    while (heap_node->next)
    {
        if (heap_node->next->signature != HEAP_SIGNATURE)
            break;
        if (heap_node->size >= size && heap_node->is_free)
            break;
        heap_node = heap_node->next;
    }
    if (heap_node->size == size)
    {
        heap_node->is_free = false;
        heap_node->align = align;
        return (void *)((uint64_t)heap_node + sizeof(heap_node_t));
    }
    else if (heap_node->size > size)
    {
        uint64_t address = NULL;
        heap_node_t *new_heap_node = (heap_node_t *)((uint64_t)heap_node + sizeof(heap_node_t) * 2 + size);
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

void *heap_mrealloc(heap_t *heap, void *ptr, uint64_t size)
{
    if (!heap || !ptr)
        return (void *)NULL;
    heap_node_t *node = (heap_node_t *)(*(uint64_t *)((uint64_t)ptr - sizeof(uint64_t)));
    if (node->signature != HEAP_SIGNATURE)
        return;
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
        return;
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
    heap_expand_free_nodes(heap);
    heap_node_t *node = (heap_node_t *)((uint64_t)ptr - sizeof(heap_node_t));
    if (node->signature != HEAP_SIGNATURE)
        return;
    if (node->next)
    {
        if (!node->next->next)
        {
            uint64_t new_size = (heap->end - heap->start) - (sizeof(heap_node_t) + node->size);
            node->size += node->next->size;
            node->next = (heap_node_t *)NULL;
            heap_contract(heap, new_size);
        }
    }
    node->is_free = true;
    heap_expand_free_nodes(heap);
}