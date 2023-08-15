#include <fs.h>

fs_node_t *fs_root;

uint32_t fs_read(fs_node_t *node, uint32_t offset, uint32_t size, void *buffer)
{
    if (node->read != 0)
        return node->read(node, offset, size, buffer);
    return NULL;
}

uint32_t fs_write(fs_node_t *node, uint32_t offset, uint32_t size, void *buffer)
{
    if (node->write != 0)
        return node->write(node, offset, size, buffer);
    return NULL;
}

void fs_open(fs_node_t *node, uint8_t flags)
{
    if (node->open)
        return node->open(node, flags);
}

void fs_close(fs_node_t *node)
{
    if (node->close)
        return node->close(node);
}

fs_dir_t *fs_readdir(fs_node_t *node, uint32_t index)
{
    if ((node->flags & 0x7) == FS_DIRECTORY && node->readdir)
        return node->readdir(node, index);
    return (fs_dir_t *)NULL;
}

fs_node_t *fs_finddir(fs_node_t *node, char *name)
{
    if ((node->flags & 0x7) == FS_DIRECTORY && node->finddir)
        return node->finddir(node, name);
    return (fs_node_t *)NULL;
}