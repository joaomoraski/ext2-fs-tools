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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


void load_super_block(ext2_info* fs_info);
void load_group_desc(ext2_info* fs_info);
void read_data_block(ext2_info* fs_info, int block_number, char* buffer, int buffer_size);
bool verify_file_exists(ext2_info* fs_info, unsigned int i_block, char* file_name);
unsigned int find_inode_number_by_path(ext2_info* fs_info, char* path);
unsigned int find_parent_inode_and_final_name(ext2_info* fs_info, const char* full_path, char* final_name_out);
unsigned int find_and_allocate_item(ext2_info* fs_info, char type);
void deallocate_item(ext2_info* fs_info, unsigned int item_number, char type);
inode_struct read_inode_by_number(ext2_info* fs_info, unsigned int inode_number);
void write_inode_by_number(ext2_info* fs_info, unsigned int inode_number, inode_struct* new_inode);
int add_dir_entry(ext2_info* fs_info, unsigned int parent_inode_num, unsigned int new_inode_num, char* filename,
                  int file_type, bool commit_changes);
int remove_dir_entry(ext2_info* fs_info, unsigned int parent_inode_num, char* filename_to_remove);


#endif //EXT2_FS_METHODS_H
