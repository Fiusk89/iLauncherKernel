#pragma once
#ifndef FS_H
#define FS_H
#include <kernel.h>
#define FS_FILE 0x01
#define FS_DIRECTORY 0x02
#define FS_CHARDEVICE 0x03
#define FS_BLOCKDEVICE 0x04
#define FS_PIPE 0x05
#define FS_SYMLINK 0x06
#define FS_MOUNTPOINT 0x07
#define FS_OPEN_READ 0x01
#define FS_OPEN_WRITE 0x02

typedef struct fs_dir
{
    char name[256];
    struct fs_dir *prev;
    struct fs_dir *next;
} fs_dir_t;

typedef struct fs_node
{
    char name[256];
    uint32_t mask;
    uint32_t uid;
    uint32_t gid;
    uint32_t flags;
    uint32_t size;
    uint32_t impl;
    uint32_t (*read)(struct fs_node *node, uint32_t offset, uint32_t size, void *buffer);
    uint32_t (*write)(struct fs_node *node, uint32_t offset, uint32_t size, void *buffer);
    void (*open)(struct fs_node *node, uint8_t flags);
    void (*close)(struct fs_node *node);
    struct fs_dir *(*read_dir)(struct fs_node *node, uint8_t *name);
    struct fs_node *(*find_dir)(struct fs_node *node, uint8_t *name);
    struct fs_node *ptr;
    struct fs_node *prev;
    struct fs_node *next;
} fs_node_t;

extern fs_node_t *fs_root;

uint32_t fs_read(fs_node_t *node, uint32_t offset, uint32_t size, void *buffer);
uint32_t fs_write(fs_node_t *node, uint32_t offset, uint32_t size, void *buffer);
void fs_open(fs_node_t *node, uint8_t flags);
void fs_close(fs_node_t *node);
fs_dir_t *fs_read_dir(fs_node_t *node, uint8_t *name);
fs_node_t *fs_find_dir(fs_node_t *node, uint8_t *name);
#endif