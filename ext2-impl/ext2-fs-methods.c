//
// Created by moraski on 07/06/25.
//
#include "ext2-fs-methods.h"

#include <stdbool.h>
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

inode_struct read_inode_by_number(ext2_info* fs_info, unsigned int inode_number) {
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

unsigned int find_inode_number_by_path(ext2_info* fs_info, char *path) {
    unsigned int start_inode;

    if (path[0] == '/') { // Caminho absoluto
        start_inode = 2;
    } else {
        start_inode = fs_info->current_dir_inode;
    }
    char* splited_path = strtok(path, "/");

    while (splited_path != NULL) {
        inode_struct inode = read_inode_by_number(fs_info, start_inode);

        if (!S_ISDIR(inode.i_mode)) { // não é um diretorio.
            printf("Erro: '%s' não é um diretório no caminho.\n", "componente_anterior"); // Melhorar isso depois
            return 0;
        }

        // Ler o data block do diretorio atual
        char tmp[1024];
        read_data_block(fs_info, inode.i_block[0], tmp, sizeof(tmp));

        char* actual_pointer = tmp;
        int bytes_read = 0;
        unsigned int next_inode = 0;

        while (bytes_read < fs_info->block_size) {
            dir_entry* entry = (dir_entry*)actual_pointer;

            // Se o tamanho do diretorio é 0 tem algo errado
            if (entry->rec_len == 0) { break; }

            // 0 significa excluido ou vazio
            if (entry->inode != 0) {
                // comparar o nome da entrada com o caminho atual e o tamanho
                if (strncmp(splited_path, entry->name, entry->name_len) == 0 && strlen(splited_path) == entry->name_len) {
                    next_inode = entry->inode;
                    break;
                }
            }
            actual_pointer += entry->rec_len;
            bytes_read += entry->rec_len;
        }

        if (next_inode == 0) {
            printf("Erro: '%s' não encontrado.\n", splited_path);
            return 0;
        }

        start_inode = next_inode;

        splited_path = strtok(NULL, "/");
    }
    return start_inode;
}


void read_data_block(ext2_info* fs_info, int block_number, char* buffer, int buffer_size) {
    int content_location = block_number * fs_info->block_size;
    lseek(fs_info->fd, content_location, SEEK_SET);
    read(fs_info->fd, buffer, buffer_size);
}
