#include <driverfs.h>

fs_node_t *fs_driver;

void driverfs_install()
{
    fs_driver = (fs_node_t *)kmalloc(sizeof(fs_node_t));
    memset(fs_driver, 0, sizeof(fs_node_t));
}