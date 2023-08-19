#pragma once
#ifndef DRIVERFS_H
#define DRIVERFS_H
#include <kernel.h>

extern struct fs_node *fs_dev;

void devfs_add_dev(struct fs_node *dev, uint8_t *name);
void devfs_install();
#endif