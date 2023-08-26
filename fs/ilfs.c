#include <kernel.h>

fs_node_t *ilfs_dev;
fs_node_t *ilfs_dir;

fs_node_t *ilfs_open(fs_node_t *node, uint8_t flags)
{
    if (!node)
        return (fs_node_t *)NULL;
    fs_node_t *ret = (fs_node_t *)kclone(node);
    ret->flags |= flags;
    return ret;
}

void ilfs_close(fs_node_t *node)
{
    kfree(node);
}

uint32_t ilfs_add_nodes(fs_node_t *list, fs_node_t *dev, char *cut_name, uint32_t offset)
{
    ilfs_node_t *node = (ilfs_node_t *)kmalloc(sizeof(ilfs_node_t));
    memset(node, 0, sizeof(ilfs_node_t));
    while (list)
    {
        fs_node_t *tmpfs_node = fs_open(fs_dev, dev->name, FS_OPEN_READ);
        fs_read(tmpfs_node, offset, sizeof(ilfs_node_t), node);
        fs_close(tmpfs_node);
        if (!node->name[0] || (strlen(cut_name) &&
                               strncmp(node->name, cut_name, strlen(cut_name) - 1)))
        {
            if (list->prev)
            {
                list = list->prev;
                kfree(list->next);
                list->next = (fs_node_t *)NULL;
            }
            else if (list->parent && !list->prev)
            {
                list = list->parent;
                kfree(list->ptr);
                list->ptr = (fs_node_t *)NULL;
            }
            break;
        }
        offset += sizeof(ilfs_node_t) + node->size;
        memcpy(list->name, node->name + strlen(cut_name), fs_cutdir(node->name + strlen(cut_name)) - 1);
        switch (node->type)
        {
        case ILFS_REG:
            list->flags = FS_FILE;
            break;
        case ILFS_DIR:
            list->flags = FS_DIRECTORY;
            list->ptr = (fs_node_t *)kmalloc(sizeof(fs_node_t));
            memset(list->ptr, 0, sizeof(fs_node_t));
            list->ptr->parent = list;
            strcat(node->name, "/");
            offset = ilfs_add_nodes(list->ptr, dev, node->name, offset);
            break;
        default:
            break;
        }
        list->next = (fs_node_t *)kmalloc(sizeof(fs_node_t));
        memset(list->next, 0, sizeof(fs_node_t));
        list->next->prev = list;
        list = list->next;
    }
    kfree(node);
    return offset;
}

fs_node_t *ilfs_list_nodes(fs_node_t *list)
{
    while (list)
    {
        kprintf("%s\n", list->name);
        list = list->next;
    }
}

fs_node_t *ilfs_create(fs_node_t *dev)
{
    if (!dev)
        return (fs_node_t *)NULL;
    uint32_t index = 0;
    while (ilfs_dir)
    {
        index++;
        ilfs_dir = ilfs_dir->next;
    }
    fs_node_t *ret = (fs_node_t *)kmalloc(sizeof(fs_node_t));
    memset(ret, 0, sizeof(fs_node_t));
    char number[255 - sizeof("ilfs")];
    memset(number, 0, 255 - sizeof("ilfs"));
    itoa(number, index, 255 - sizeof("ilfs"));
    strcat(ret->name, "ilfs");
    strcat(ret->name, number);
    ret->flags = FS_DIRECTORY;
    ret->ptr = (fs_node_t *)kmalloc(sizeof(fs_node_t));
    memset(ret->ptr, 0, sizeof(fs_node_t));
    uint64_t offset = sizeof(ilfs_header_t);
    ilfs_add_nodes(ret->ptr, dev, "\0", offset);
    ilfs_list_nodes(ret->ptr);
    return ret;
}