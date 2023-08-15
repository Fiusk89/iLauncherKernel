#pragma once
#ifndef DRIVERFS_H
#define DRIVERFS_H
#include <kernel.h>

extern struct fs_node *fs_driver;

void driverfs_install();
#endif