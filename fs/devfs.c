#include <devfs.h>

fs_node_t *fs_dev;

uint8_t *charmap = "0123456789QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm";

void devfs_add_dev(fs_node_t *dev, uint8_t *name)
{
    if (!dev || !name || !strlen(name))
        return;
    if (!((dev->flags & 0x7) == FS_BLOCKDEVICE || (dev->flags & 0x7) == FS_CHARDEVICE))
        return;
    if (strlen(name) > 255 - strlen("dev/"))
        return;
    uint32_t pass = 0;
    for (uint32_t i = 0; name[i] != '\0'; i++)
        for (uint8_t charmap_i = 0; charmap[charmap_i] != '\0'; charmap_i++)
            if (name[i] == charmap[charmap_i])
                pass++;
    if (pass != strlen(name))
        return;
    uint8_t new_name[4096];
    memset(new_name, 0, 4096);
    strcat(new_name, "dev/");
    strcat(new_name, name);
    memcpy(dev->name, new_name, strlen(new_name));
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
    fs_dev->flags = FS_DIRECTORY;
}