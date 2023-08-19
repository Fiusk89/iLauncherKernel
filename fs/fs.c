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

fs_node_t *fs_open(fs_node_t *node, uint8_t *name, uint8_t flags)
{
    if (!name || !strlen(name))
        return (fs_node_t *)NULL;
    if (*name == '/')
        node = fs_root, name++;
    if (!node || !flags || !strlen(name))
        return (fs_node_t *)NULL;
    while (node)
    {
        if (!strcmp(node->name, name))
            break;
        node = node->next;
    }
    if (!node)
        return (fs_node_t *)NULL;
    if (node->open)
        return node->open(node, name, flags);
}

void fs_close(fs_node_t *node)
{
    if (!node)
        return;
    if (node->close)
        return node->close(node);
}