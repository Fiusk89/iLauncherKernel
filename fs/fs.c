#include <fs.h>

fs_node_t *fs_root;

uint32_t fs_cutdir(char *name)
{
    uint32_t i = 0;
    while (name[i])
    {
        if (name[i] == '/')
            break;
        i++;
    }
    return i + 1;
}

void fs_cut_slashes(char *name)
{
    for (uint32_t i = 0; name[i]; i++)
    {
        for (uint32_t j = i + 1; name[j]; j++)
        {
            if (name[i] == '/' && name[j] == '/')
            {
                for (uint32_t k = j; name[k]; k++)
                {
                    name[k] = name[k + 1];
                }
                j = i;
            }
        }
    }
}

bool fs_contains_slash(char *name)
{
    for (uint32_t i = 0; name[i]; i++)
    {
        if (name[i] == '/' && name[i + 1])
            return 1;
    }
    return 0;
}

uint32_t fs_count_slashes(char *name)
{
    uint32_t i = 0, j = 0;
    while (name[i])
    {
        if (name[i] == '/')
            j++;
        i++;
    }
    return j;
}

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
    if ((node->flags & 0x7) == FS_DIRECTORY)
    {
        if (!name || !strlen(name))
            return (fs_node_t *)NULL;
        if (*name == '/')
            node = fs_root, name++;
        if (!node || !flags || !strlen(name))
            return (fs_node_t *)NULL;
        fs_cut_slashes(name);
        uint8_t *tmp1 = name;
        node = node->ptr;
        while (node)
        {
            if ((node->flags & 0x7) == FS_DIRECTORY)
                if (!strncmp(name, node->name, strlen(node->name) - 1))
                    node = node->ptr, tmp1 += fs_cutdir(tmp1);
            if (!fs_contains_slash(tmp1))
                if (!strncmp(name, node->name, strlen(node->name) - 1))
                    break;
            tmp1 += fs_cutdir(tmp1);
            node = node->next;
        }
    }
    if (!node)
        return (fs_node_t *)NULL;
    if (node->open)
        return node->open(node, flags);
}

void fs_close(fs_node_t *node)
{
    if (!node)
        return;
    if (node->close)
        return node->close(node);
}