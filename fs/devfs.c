#include <devfs.h>

fs_node_t *fs_dev;
fs_dir_t *fs_dev_dir;

uint8_t *charmap = "QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm";

fs_node_t *devfs_read_dir(fs_node_t *node, uint8_t *name)
{
}

fs_node_t *devfs_find_dir(fs_node_t *node, uint8_t *name)
{
}

void devfs_add_dev(fs_node_t *dev, uint8_t *name)
{
    if (!dev || !name || !strlen(name))
        return;
    if ((dev->flags & 0x7) != FS_DIRECTORY)
        return;
    if (strlen(name) > 255 - strlen("dev/"))
        return;
    uint16_t pass = 0;
    for (uint16_t i = 0; name[i] != '\0'; i++)
        for (uint8_t charmap_i = 0; charmap[charmap_i] != '\0'; charmap_i++)
            if (name[i] == charmap[charmap_i])
                pass++;
    if (pass != strlen(name))
        return;
    fs_node_t *node = (fs_node_t *)kmalloc(sizeof(fs_node_t));
    memset(node, 0, sizeof(fs_node_t));
    strcat(node->name, "dev/");
    strcat(node->name, name);
    node->flags = FS_MOUNTPOINT;
    node->ptr = dev;
    fs_dir_t *node_dir = (fs_dir_t *)kmalloc(sizeof(fs_dir_t));
    memset(node_dir, 0, sizeof(fs_node_t));
    strcat(node_dir->name, "dev/");
    strcat(node_dir->name, name);
    fs_node_t *tmp = fs_dev;
    while (tmp->next)
        tmp = tmp->next;
    node->prev = tmp;
    tmp->next = node;
    fs_dir_t *tmp_dir = fs_dev_dir;
    while (tmp_dir->next)
        tmp_dir = tmp_dir->next;
    node_dir->prev = tmp_dir;
    tmp_dir->next = node_dir;
}

void devfs_remove_dev(fs_node_t *dev)
{
    fs_node_t *tmp = fs_dev;
    while (tmp->next)
    {
        if (tmp == dev)
            break;
        tmp = tmp->next;
    }
    tmp->next->prev = tmp->prev;
    tmp->prev->next = tmp->next;
}

void devfs_install()
{
    fs_dev = (fs_node_t *)kmalloc(sizeof(fs_node_t));
    memset(fs_dev, 0, sizeof(fs_node_t));
    strcpy(fs_dev->name, "dev");
    fs_dev->read_dir = devfs_read_dir;
    fs_dev->find_dir = devfs_find_dir;
    fs_dev->flags = FS_DIRECTORY;
    fs_dev_dir = (fs_dir_t *)kmalloc(sizeof(fs_dir_t));
    memset(fs_dev_dir, 0, sizeof(fs_dir_t));
    strcpy(fs_dev_dir->name, "dev");
}