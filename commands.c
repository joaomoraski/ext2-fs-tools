//
// Created by moraski on 07/06/25.
//

#include <stdio.h>
#include <stdlib.h>

#include "ext2-impl/ext2_structs.h"

void info(const ext2_info fs_info) {
    super_block super_block = fs_info.sb;
    unsigned int block_size = fs_info.block_size;
    printf("Volume name.....: %s\n"
           "Image size......: %u\n"
           "Free space......: %u\n"
           "Free inodes.....: %u\n"
           "Free blocks.....: %u\n"
           "Block size......: %u\n"
           "Inode size......: %u\n"
           "Groups count....: %u\n"
           "Groups size.....: %u\n"
           "Groups inodes...: %u\n"
           "Inodetable size.: %u\n",
           (char*)super_block.s_volume_name, // funciona sem o cast, mas quis tirar highlight da IDE
           super_block.s_blocks_count * super_block.s_log_block_size,
           super_block.s_free_blocks_count * block_size / 1024,
           super_block.s_free_inodes_count,
           super_block.s_free_blocks_count,
           block_size,
           super_block.s_inode_size,
           super_block.s_blocks_count / super_block.s_blocks_per_group,
           super_block.s_blocks_per_group,
           super_block.s_inodes_per_group,
           (super_block.s_inodes_per_group * super_block.s_inode_size) / block_size
    );
}

void ls(const ext2_info fs_info) {
    printf("ls\n");
}

