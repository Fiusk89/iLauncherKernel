#include <ilfs.h>

ilfs_header_t *ilrdfs_header;

int8_t ilfs_ilrdfs()
{
    if (!ilrdfs_header)
        return -1;
    if (!strcmp(ilrdfs_header->signature, "ilfs"))
        return false;
    else if (!strcmp(ilrdfs_header->signature, "ilrdfs"))
        return true;
    else
        return -1;
}

ilfs_node_t *ilrdfs_find_node(uint8_t *name)
{
    if (ilfs_ilrdfs() != true)
        return NULL;
    uint8_t *ilrdfs_node = (uint32_t)ilrdfs_header + sizeof(ilfs_header_t);
    while (*ilrdfs_node)
    {
        ilfs_node_t *node = ilrdfs_node;
        if (!strcmp(node->name, name))
            return node;
        ilrdfs_node += (uint32_t)node->size + sizeof(ilfs_node_t) + 1;
    }
    return NULL;
}

ilfs_node_t *ilrdfs_read_file(uint8_t *name)
{
    if (ilfs_ilrdfs() != true)
        return NULL;
    ilfs_node_t *node = ilrdfs_find_node(name);
    uint8_t *node_data = (uint32_t *)ilfs_skip_node_header(node);
    if (!node)
        return NULL;
    ilfs_node_t *ret = kmalloc(node->real_size + sizeof(ilfs_node_t) + 1);
    memset(ret, 0, node->real_size + sizeof(ilfs_node_t) + 1);
    memcpy(ret, node, sizeof(ilfs_node_t));
    if (!ret->compressed)
    {
        memcpy((void *)((uint32_t)ret + sizeof(ilfs_node_t)), (void *)node_data, ret->real_size);
        return ret;
    }
    if (!ret->size)
        return ret;
    rle_decompress((uint32_t)ret + sizeof(ilfs_node_t), node_data, node->size);
    return ret;
}

void ilrdfs_list_nodes()
{
    uint8_t *ilrdfs_node = (uint32_t)ilrdfs_header + sizeof(ilfs_header_t);
    while (*ilrdfs_node)
    {
        ilfs_node_t *node = ilrdfs_node;
        kprintf("%s\n", node->name);
        // sleep(PIT_HZ / 4);
        ilrdfs_node += (uint32_t)node->size + sizeof(ilfs_node_t) + 1;
    }
}

void ilrdfs_install(uint32_t start)
{
    ilrdfs_header = start;
    if (ilfs_ilrdfs() == -1)
    {
        kprintf("invalid RAMDISK file\n");
        while (1)
            ;
    }
}