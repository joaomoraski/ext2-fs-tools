#ifndef COMMANDS_H
#define COMMANDS_H

#include "../ext2-impl/ext2_structs.h"

void info(ext2_info fs_info);
void ls(ext2_info fs_info, char* path);
void cd(ext2_info* fs_info, char* path);
void attr(ext2_info* fs_info, char* path);
void cat(ext2_info* fs_info, char* path);
void touch(ext2_info* fs_info, char* path_to_file);
void cmd_mkdir(ext2_info* fs_info, char* path);
void multi_touch(ext2_info* fs_info, char** args, int argc);
void multi_cmd_mkdir(ext2_info* fs_info, char** args, int argc);
void rm(ext2_info* fs_info, char* path);
void cmd_rmdir(ext2_info* fs_info, char* path);
void multi_rm(ext2_info* fs_info, char** args, int argc);
void multi_cmd_rmdir(ext2_info* fs_info, char** args, int argc);
int cp(ext2_info* fs_info, char* source_path, char* target_path);
void mv(ext2_info* fs_info, char* source_path, char* target_path);
void cmd_rename(ext2_info* fs_info, char* source_name, char* new_name);
void print_superblock(ext2_info fs_info);
void print_groups(ext2_info fs_info);
void print_inode(ext2_info fs_info, int inode_number);

#endif