#include <fs.h>

fs_node_t *fs_root;

uint32_t fs_read(fs_node_t *node, uint32_t offset, uint32_t size, void *buffer)
{
    if (node->read)
        return node->read(node, offset, size, buffer);
    return NULL;
}

uint32_t fs_write(fs_node_t *node, uint32_t offset, uint32_t size, void *buffer)
{
    if (node->write)
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

fs_dir_t *fs_read_dir(fs_node_t *node, uint32_t index)
{
    if (node == fs_root && index < 1)
        return fs_dev_dir;
    if ((node->flags & 0x7) == FS_DIRECTORY && node->read_dir)
        return node->read_dir(node, index);
    return (fs_dir_t *)NULL;
}

fs_node_t *fs_find_dir(fs_node_t *node, uint8_t *name)
{
    if (*name == '/')
        node = fs_root, name++;
    if (node == fs_root && strcmp(name, "dev"))
        return fs_dev;
    if ((node->flags & 0x7) == FS_DIRECTORY && node->find_dir)
        return node->find_dir(node, name);
    return (fs_node_t *)NULL;
}