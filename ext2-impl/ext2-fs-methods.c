//
// Created by moraski on 07/06/25.
//
#include "ext2-fs-methods.h"

#include <string.h>

void load_super_block(ext2_info* fs_info) {
    super_block super_block;

    lseek(fs_info->fd, BASE_BLOCK, SEEK_SET);
    read(fs_info->fd, &super_block, sizeof(super_block));

    if (super_block.s_magic != EXT2_SUPER_MAGIC) {
        fprintf(stderr, "Not an ext2fs filesystem\n");
        exit(EXIT_FAILURE);
    }

    fs_info->sb = super_block;
    fs_info->block_size = 1024 << super_block.s_log_block_size;
}

void load_group_desc(ext2_info* fs_info) {
    unsigned int aux = fs_info->sb.s_blocks_count + fs_info->sb.s_blocks_per_group - 1;
    fs_info->num_block_groups = aux / fs_info->sb.s_blocks_per_group; // aux foi feito so pra ficar mais facil de ler

    // alocar a memoria para os descritores de grupo
    fs_info->group_desc_array = (group_desc*)malloc(fs_info->num_block_groups * sizeof(group_desc));
    if (fs_info->group_desc_array == NULL) {
        perror("Failed to allocate memory for group descriptors");
        close(fs_info->fd);
        exit(EXIT_FAILURE);
    }

    // ler os descritores de grupo
    // começa logo apos o superbloco, entao block_size * 2
    off_t group_desc_table_offset = fs_info->block_size * 2;
    lseek(fs_info->fd, group_desc_table_offset, SEEK_SET);
    read(fs_info->fd, fs_info->group_desc_array, fs_info->num_block_groups * sizeof(group_desc));

    // definir diretorio raiz
    fs_info->current_dir_inode = 2; // inode 2 é sempre o diretório raiz
    strcpy(fs_info->current_path, "/");
}

inode_struct read_inode_by_number(ext2_info* fs_info, int inode_number) {
    int group = (inode_number - 1) / fs_info->sb.s_inodes_per_group;
    group_desc group_desc = fs_info->group_desc_array[group];
    int inode_index_on_group = (inode_number - 1) % fs_info->sb.s_inodes_per_group;
    int initial_position_inode_table = group_desc.bg_inode_table * fs_info->block_size;
    int final_position_of_inode = initial_position_inode_table + (inode_index_on_group * fs_info->sb.s_inode_size);

    inode_struct inode;
    lseek(fs_info->fd, final_position_of_inode, SEEK_SET);
    read(fs_info->fd, &inode, sizeof(inode_struct));
    return inode;
}

void read_data_block(ext2_info* fs_info, int block_number, char* buffer) {
    int content_location = block_number * fs_info->block_size;
    lseek(fs_info->fd, content_location, SEEK_SET);
    read(fs_info->fd, buffer, sizeof(buffer));
}
