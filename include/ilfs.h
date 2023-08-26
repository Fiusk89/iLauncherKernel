#ifndef ILFS_H
#define ILFS_H
#ifndef KERNEL_H
#include "global.h"
#else
#include <kernel.h>
#endif

typedef enum ilfs_type
{
    ILFS_REG = 0,
    ILFS_DIR = 1,
} ilfs_type_t;

typedef struct ilfs_header
{
    uint8_t signature[8];
    uint8_t name[256];
    uint8_t date[16];
    uint8_t version[16];
    uint64_t size;
} __attribute__((packed)) ilfs_header_t;

typedef struct ilfs_node
{
    uint8_t name[256];
    uint8_t link_name[256];
    uint8_t date[16];
    uint8_t type;
    uint8_t compressed;
    uint64_t size, real_size;
} __attribute__((packed)) ilfs_node_t;

static inline void *ilfs_skip_node_header(ilfs_node_t *node)
{
    return (void *)((uint64_t)node + sizeof(ilfs_node_t));
}

#ifdef KERNEL_H
struct fs_node *ilfs_create(uint8_t *dev);
#endif

#ifndef KERNEL_H

static uint64_t ilfs_get_free_node(uint8_t *ptr)
{
    if (!ptr)
        return 0;
    ilfs_header_t *header = (ilfs_header_t *)ptr;
    ptr += sizeof(ilfs_header_t);
    while (*ptr)
    {
        ilfs_node_t *node = (ilfs_node_t *)ptr;
        ptr += node->size + sizeof(ilfs_node_t);
    }
    return (uint64_t)ptr;
}

static uint8_t ilfs_add_node(void *ptr, ilfs_node_t *node, void *data)
{
    if (!ptr || !node)
        return 1;
    if (!data)
        node->size = 0, node->real_size = 0;
    ilfs_header_t *header = (ilfs_header_t *)ptr;
    if (node->compressed)
        node->size = rle_get_size(data, node->real_size);
    uint64_t address = ilfs_get_free_node((uint8_t *)ptr);
    if (!address)
        return 1;
    if ((address - (uint64_t)ptr) + (!node->compressed ? node->real_size : node->size) + sizeof(ilfs_node_t) > header->size)
        return 1;
    if (!node->compressed)
    {
        node->size = node->real_size;
        memcpy((void *)address, (void *)node, sizeof(ilfs_node_t));
        if (data)
            memcpy((void *)(address + sizeof(ilfs_node_t)), (void *)data, node->real_size);
        return 0;
    }
    memcpy((void *)address, (void *)node, sizeof(ilfs_node_t));
    if (data)
        rle_compress((void *)(address + sizeof(ilfs_node_t)), (void *)data, node->real_size);
    return 0;
}
#endif
#endif