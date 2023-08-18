#pragma once
#ifndef DRIVERFS_H
#define DRIVERFS_H
#include <kernel.h>

extern struct fs_node *fs_dev;

void devfs_install();
#endif