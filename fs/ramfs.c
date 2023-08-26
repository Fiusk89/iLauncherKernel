#include <ramfs.h>

typedef struct ramdisk
{
    uint16_t index;
    fs_node_t *dev;
    void *ramdisk_start;
    uint64_t ramdisk_size;
    struct ramdisk *prev;
    struct ramdisk *next;
} ramdisk_t;

ramdisk_t *fs_ramdisk;

uint32_t ramfs_read(fs_node_t *node, uint32_t offset, uint32_t size, void *buffer)
{
    if (!node || !node->ptr)
        return 3;
    if (~node->flags & FS_OPEN_READ)
        return 2;
    ramdisk_t *tmp = fs_ramdisk;
    while (tmp)
    {
        if (tmp->dev == node->ptr)
            break;
        tmp = tmp->next;
    }
    if (tmp)
    {
        if (tmp->dev->size < offset)
            return 4;
        memcpy(buffer, tmp->ramdisk_start + offset, limit(size, node->ptr->size - offset));
        return 0;
    }
    return 1;
}

uint32_t ramfs_write(fs_node_t *node, uint32_t offset, uint32_t size, void *buffer)
{
    if (!node || !node->ptr)
        return 3;
    if (~node->flags & FS_OPEN_WRITE)
        return 2;
    ramdisk_t *tmp = fs_ramdisk;
    while (tmp)
    {
        if (tmp->dev == node->ptr)
            break;
        tmp = tmp->next;
    }
    if (tmp)
    {
        node->boffset = offset;
        node->bsize = size;
        node->buffer = buffer;
        return 0;
    }
    return 1;
}

fs_node_t *ramfs_open(fs_node_t *node, uint8_t flags)
{
    if (!node)
        return (fs_node_t *)NULL;
    fs_node_t *ret = (fs_node_t *)kclone(node);
    if (flags)
        ret->flags |= flags;
    ret->ptr = node;
    return ret;
}

void ramfs_close(fs_node_t *node)
{
    if (!node || !node->ptr)
        return;
    if (node->flags & FS_OPEN_WRITE && node->buffer)
    {
        ramdisk_t *tmp = fs_ramdisk;
        while (tmp)
        {
            if (tmp->dev == node->ptr)
                break;
            tmp = tmp->next;
        }
        if (tmp)
            memcpy(tmp->ramdisk_start + node->boffset, node->buffer, node->bsize);
    }
    kfree(node);
}

void ramfs_add(void *start, void *end)
{
    ramdisk_t *tmp = fs_ramdisk;
    if (tmp)
    {
        while (tmp->next)
            tmp = tmp->next;
        tmp->next = (ramdisk_t *)kmalloc(sizeof(ramdisk_t));
        memset(tmp->next, 0, sizeof(ramdisk_t));
        tmp->next->prev = tmp;
        tmp->next->index = tmp->index + 1;
        tmp->next->ramdisk_start = start;
        tmp->next->ramdisk_size = (uint64_t)end - (uint64_t)start;
        tmp = tmp->next;
    }
    else
    {
        tmp = (ramdisk_t *)kmalloc(sizeof(ramdisk_t));
        memset(tmp, 0, sizeof(ramdisk_t));
        tmp->ramdisk_start = start;
        tmp->ramdisk_size = (uint64_t)end - (uint64_t)start;
        fs_ramdisk = tmp;
    }
    tmp->dev = (fs_node_t *)kmalloc(sizeof(fs_node_t));
    memset(tmp->dev, 0, sizeof(fs_node_t));
    char number[255 - sizeof("ramdisk")];
    memset(number, 0, 255 - sizeof("ramdisk"));
    itoa(number, tmp->index, 255 - sizeof("ramdisk"));
    strcat(tmp->dev->name, "ramdisk");
    strcat(tmp->dev->name, number);
    tmp->dev->open = ramfs_open;
    tmp->dev->close = ramfs_close;
    tmp->dev->read = ramfs_read;
    tmp->dev->write = ramfs_write;
    tmp->dev->flags = FS_BLOCKDEVICE;
    tmp->dev->size = (uint32_t)end - (uint32_t)start;
    devfs_add_dev(tmp->dev, tmp->dev->name);
}