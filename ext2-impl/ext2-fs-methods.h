//
// Created by moraski on 07/06/25.
//
#ifndef EXT2_FS_METHODS_H
#define EXT2_FS_METHODS_H

#include "ext2-fs-methods.h"
#include "ext2_structs.h"
#include "ext2-defs.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

void load_super_block(ext2_info* fs_info);

#endif //EXT2_FS_METHODS_H
