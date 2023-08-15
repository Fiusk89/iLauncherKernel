#pragma once
#ifndef ILFS_H
#define ILFS_H
#include <kernel.h>

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

typedef struct ilfs_simple_compress8
{
    uint8_t header_size;
    uint8_t size;
    uint8_t value;
} __attribute__((packed)) ilfs_simple_compress8_t;

typedef struct ilfs_simple_compress16
{
    uint8_t header_size;
    uint16_t size;
    uint8_t value;
} __attribute__((packed)) ilfs_simple_compress16_t;

typedef struct ilfs_simple_compress32
{
    uint8_t header_size;
    uint32_t size;
    uint8_t value;
} __attribute__((packed)) ilfs_simple_compress32_t;

typedef struct ilfs_simple_compress64
{
    uint8_t header_size;
    uint64_t size;
    uint8_t value;
} __attribute__((packed)) ilfs_simple_compress64_t;

static inline void *ilfs_skip_node_header(ilfs_node_t *node)
{
    return (void *)((uint64_t)node + sizeof(ilfs_node_t));
}

ilfs_node_t *ilrdfs_read_file(uint8_t *name);
void ilrdfs_install(uint32_t start);
#endif