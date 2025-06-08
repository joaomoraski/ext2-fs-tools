//
// Created by moraski on 07/06/25.
//
#include "ext2-fs-methods.h"

#include <string.h>

void load_super_block(ext2_info* fs_info) {
    super_block super_block;
    int fd;

    if ((fd = open(IMG_PATH, O_RDONLY)) < 0) {
        perror(IMG_PATH);
        exit(EXIT_FAILURE);
    }

    lseek(fd, BASE_BLOCK, SEEK_SET);
    read(fd, &super_block, sizeof(super_block));
    close(fd);

    if (super_block.s_magic != EXT2_SUPER_MAGIC) {
        fprintf(stderr, "Not an ext2fs filesystem\n");
        exit(EXIT_FAILURE);
    }

    fs_info->sb = super_block;
    fs_info->block_size = 1024 << super_block.s_log_block_size;

    close(fd);
}

void load_group_desc(ext2_info* fs_info) {
    int fd;

    if ((fd = open(IMG_PATH, O_RDONLY)) < 0) {
        perror(IMG_PATH);
        exit(EXIT_FAILURE);
    }

    unsigned int aux = fs_info->sb.s_blocks_count + fs_info->sb.s_blocks_per_group - 1;
    fs_info->num_block_groups = aux / fs_info->sb.s_blocks_per_group; // aux foi feito so pra ficar mais facil de ler

    // alocar a memoria para os descritores de grupo
    fs_info->group_desc_array = (group_desc*) malloc(fs_info->num_block_groups * sizeof(group_desc));
    if (fs_info->group_desc_array == NULL) {
        perror("Failed to allocate memory for group descriptors");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // ler os descritores de grupo
    // começa logo apos o superbloco, entao block_size * 2
    off_t group_desc_table_offset = fs_info->block_size * 2;
    lseek(fd, group_desc_table_offset, SEEK_SET);
    read(fd, fs_info->group_desc_array, fs_info->num_block_groups * sizeof(group_desc));

    // definir diretorio raiz
    fs_info->current_dir_inode = 2; // inode 2 é sempre o diretório raiz
    strcpy(fs_info->current_path, "/");
}

