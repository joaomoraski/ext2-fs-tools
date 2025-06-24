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
#include <sys/stat.h>


void load_super_block(ext2_info* fs_info);
void load_group_desc(ext2_info* fs_info);
inode_struct read_inode_by_number(ext2_info* fs_info, unsigned int inode_number);
unsigned int find_inode_number_by_path(ext2_info* fs_info, char *path);
void read_data_block(ext2_info* fs_info, int block_number, char *buffer, int buffer_size);

#endif //EXT2_FS_METHODS_H
