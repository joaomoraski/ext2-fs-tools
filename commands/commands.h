#ifndef COMMANDS_H
#define COMMANDS_H

#include "../ext2-impl/ext2_structs.h"

void info(ext2_info fs_info);
void ls(ext2_info fs_info);
void print_superblock(ext2_info fs_info);
void print_groups(ext2_info fs_info);
void print_inode(ext2_info fs_info, int inode_number);

#endif