#include <devfs.h>

fs_node_t *fs_dev;
fs_dir_t *fs_dev_dir;

uint8_t *charmap = "QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm";

void devfs_add_dev(fs_node_t *dev, uint8_t *name)
{
    if (!dev || !name || !strlen(name))
        return;
    if ((dev->flags & 0x7) != FS_DIRECTORY)
        return;
    uint16_t pass = 0;
    for (uint16_t i = 0; name[i] != '\0'; i++)
        for (uint8_t charmap_i = 0; charmap[charmap_i] != '\0'; charmap_i++)
            if (name[i] == charmap[charmap_i])
                pass++;
    if (pass != strlen(name))
        return;
    fs_node_t *tmp = fs_dev;
    while (tmp->next)
        tmp = tmp->next;
    dev->prev = tmp;
    tmp->next = dev;
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
    fs_dev_dir = (fs_dir_t *)kmalloc(sizeof(fs_dir_t));
    memset(fs_dev_dir, 0, sizeof(fs_dir_t));
    strcpy(fs_dev_dir->name, "dev");
    fs_dev_dir->inode = 0;
}