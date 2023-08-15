#pragma once
#ifndef FS_H
#define FS_H
#include <ctype.h>
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
    uint32_t index;
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
    uint32_t inode;
    uint32_t length;
    uint32_t impl;
    uint32_t (*read)(struct fs_node *, uint32_t, uint32_t, void *);
    uint32_t (*write)(struct fs_node *, uint32_t, uint32_t, void *);
    void (*open)(struct fs_node *, uint8_t);
    void (*close)(struct fs_node *);
    struct fs_dir *(*readdir)(struct fs_node *, uint32_t);
    struct fs_node *(*finddir)(struct fs_node *, char *name);
    struct fs_node *prev;
    struct fs_node *next;
} fs_node_t;

extern fs_node_t *fs_root;

uint32_t fs_read(fs_node_t *node, uint32_t offset, uint32_t size, void *buffer);
uint32_t fs_write(fs_node_t *node, uint32_t offset, uint32_t size, void *buffer);
void fs_open(fs_node_t *node, uint8_t flags);
void fs_close(fs_node_t *node);
fs_dir_t *fs_readdir(fs_node_t *node, uint32_t index);
fs_node_t *fs_finddir(fs_node_t *node, char *name);
#endif