#include <fs.h>

fs_node_t *fs_root = 0;

uint32_t read_fs(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer)
{
    if (node->read != 0)
        return node->read(node, offset, size, buffer);
    else
        return 0;
}

uint32_t write_fs(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer)
{
    if (node->write != 0)
        return node->write(node, offset, size, buffer);
    else
        return 0;
}

void open_fs(fs_node_t *node, uint8_t read, uint8_t write)
{
    if (node->open != 0)
        return node->open(node);
}

void close_fs(fs_node_t *node)
{
    if (node->close != 0)
        return node->close(node);
}

fs_dirent_t *readdir_fs(fs_node_t *node, uint32_t index)
{
    if ((node->flags & 0x7) == FS_DIRECTORY && node->readdir != 0)
        return node->readdir(node, index);
    else
        return 0;
}

fs_node_t *finddir_fs(fs_node_t *node, char *name)
{
    if ((node->flags & 0x7) == FS_DIRECTORY && node->finddir != 0)
        return node->finddir(node, name);
    else
        return 0;
}

void fs_install()
{
    fs_root = (fs_node_t *)kmalloc(sizeof(fs_node_t));
    memset(fs_root, 0, sizeof(fs_node_t));
    strcpy(fs_root->name, "/");
    fs_root->mask = fs_root->uid = fs_root->gid = fs_root->inode = fs_root->length = 0;
    fs_root->flags = FS_DIRECTORY;
    fs_root->read = 0;
    fs_root->write = 0;
    fs_root->open = 0;
    fs_root->close = 0;
    fs_root->readdir = 0;
    fs_root->finddir = 0;
    fs_root->ptr = 0;
    fs_root->impl = 0;
}