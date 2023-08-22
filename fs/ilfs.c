#include <kernel.h>

fs_node_t *ilfs_list;

fs_node_t *ilfs_create(fs_node_t *dev)
{
    if (!dev)
        return (fs_node_t *)NULL;
    uint32_t index = 0;
    while (ilfs_list)
    {
        index++;
        ilfs_list = ilfs_list->next;
    }
    fs_node_t *ret = (fs_node_t *)kmalloc(sizeof(fs_node_t));
    memset(ret, 0, sizeof(fs_node_t));
    char number[255 - sizeof("ilfs")];
    memset(number, 0, 255 - sizeof("ilfs"));
    itoa(number, index, 255 - sizeof("ilfs"));
    strcat(ret->name, "ilfs");
    strcat(ret->name, number);
    ret->flags = FS_DIRECTORY;
    ret->ptr = dev;
    uint64_t offset = sizeof(ilfs_header_t);
    ilfs_node_t *node = (ilfs_node_t *)kmalloc(sizeof(ilfs_node_t));
    fs_node_t *tmp = ret;
    while (true)
    {
        fs_node_t *tmpfs_node = fs_open(fs_dev, dev->name, FS_OPEN_READ);
        fs_read(tmpfs_node, offset, sizeof(ilfs_node_t), node);
        fs_close(tmpfs_node);
        if (!node->name[0])
            break;
        tmp->next = (fs_node_t *)kmalloc(sizeof(fs_node_t));
        memset(tmp->next, 0, sizeof(fs_node_t));
        tmp->prev = tmp;
        tmp = tmp->next;
        memcpy(tmp->name, node->name, sizeof(node->name));
        switch (node->type)
        {
        case ILFS_REG:
            tmp->flags = FS_FILE;
            break;
        case ILFS_DIR:
            tmp->flags = FS_DIRECTORY;
            break;
        default:
            break;
        }
        tmp->size = node->size;
        offset += node->size + sizeof(ilfs_node_t);
    }
    fs_node_t *test = ret;
    while (test)
    {
        kprintf("%s\n", test->name);
        test = test->next;
    }
    return ret;
}