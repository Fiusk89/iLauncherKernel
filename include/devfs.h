#pragma once
#ifndef DRIVERFS_H
#define DRIVERFS_H
#include <kernel.h>

extern struct fs_node *fs_dev;
extern struct fs_dir *fs_dev_dir;

void devfs_install();
#endif