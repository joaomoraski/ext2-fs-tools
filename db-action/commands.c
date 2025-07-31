//
// Created by moraski on 07/06/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../utils/utils.h"
#include "../ext2-impl/ext2-fs-methods.h"
#include "../ext2-impl/ext2_structs.h"
#include "../entities_struct/entities.h"

void db_insert(ext2_info* fs_info, char* path) {
    char path_copy[1024];
    strcpy(path_copy, path);
    char filename[256];
    unsigned int parent_inode_num = find_parent_inode_and_filename(fs_info, path, filename);
    if (parent_inode_num == 0) return;

    strcpy(path_copy, path);
    unsigned int inode_num = find_inode_number_by_path(fs_info, path_copy);

    if (inode_num == 0) {
        printf("Erro: tabela %s não existe", filename);
        return;
    }

    inode_struct target_inode = read_inode_by_number(fs_info, inode_num);
    append_stream_to_file(fs_info, &target_inode, sizeof(UserRecord));

    printf("O registro foi salvo na tabela.");
}

void db_select_all(ext2_info* fs_info, char* path, int limit) {
    char path_copy[1024];
    strcpy(path_copy, path);
    char filename[256];
    unsigned int parent_inode_num = find_parent_inode_and_filename(fs_info, path, filename);
    if (parent_inode_num == 0) return;

    strcpy(path_copy, path);
    unsigned int inode_num = find_inode_number_by_path(fs_info, path_copy);

    if (inode_num == 0) {
        printf("Erro: tabela %s não existe", filename);
        return;
    }

    inode_struct target_inode = read_inode_by_number(fs_info, inode_num);

    long total_bytes_to_read = target_inode.i_size;
    long bytes_read = 0;
    int record_size = sizeof(UserRecord);
    int limit_counter = limit;

    char block_buffer[fs_info->block_size];

    bool read_done = false;

    // apenas lidando com 12 entradas
    // 13 e 14 sao ponteiros indiretos
    for (int i = 0; i < 12; ++i) {
        unsigned int block_number = target_inode.i_block[i];

        if (block_number == 0) continue;

        read_data_block(fs_info, block_number, block_buffer, fs_info->block_size);

        parse_and_print_records(block_buffer, fs_info->block_size, &total_bytes_to_read, record_size, &limit_counter);

        if (total_bytes_to_read <= 0 || limit_counter == 0) {
            read_done = true;
            break;
        }
    }

    // bloco indireto
    // contem uma lista de ponteiros para outros blocos de dados
    // inode -> bloco de ponteiros -> bloco de dados
    if (!read_done && target_inode.i_block[12] != 0) {
        // Este bloco contém uma lista de ponteiros para blocos de dados.
        // 256 ponteiros cada pointer tem 4 bytes, 1024/4 = 256
        unsigned int pointers_block[256];
        read_data_block(fs_info, target_inode.i_block[12], (char*)pointers_block, fs_info->block_size);

        for (int i = 0; i < 256; i++) {
            unsigned int block_number = pointers_block[i];
            if (block_number == 0) continue; // ignora este bloco

            read_data_block(fs_info, block_number, block_buffer, fs_info->block_size);

            parse_and_print_records(block_buffer, fs_info->block_size, &total_bytes_to_read, record_size, &limit_counter);

            if (total_bytes_to_read <= 0 || limit_counter == 0) {
                read_done = true;
                break;
            }
        }
    }

    // bloco indireto duplo
    // aponta para um bloco que contem a lista de ponteiros
    // e cada um desses ponteiros aponta para outro bloco
    // inode -> bloco de ponteiros 1 -> bloco de ponteiros 2 -> bloco de dados
    if (!read_done && target_inode.i_block[13] != 0) {
        // ponteiros level 1
        unsigned int lv1_pointers[256];
        read_data_block(fs_info, target_inode.i_block[13], (char*)lv1_pointers, fs_info->block_size);

        // loop nos ponteiros
        for (int i = 0; i < 256; i++) {
            if (lv1_pointers[i] == 0) continue;

            unsigned int lv2_pointers[256];
            read_data_block(fs_info, lv1_pointers[i], (char*)lv2_pointers, fs_info->block_size);

            for (int j = 0; j < 256; j++) {
                unsigned int block_number = lv2_pointers[j];
                if (block_number == 0) continue;

                read_data_block(fs_info, block_number, block_buffer, fs_info->block_size);

                parse_and_print_records(block_buffer, fs_info->block_size, &total_bytes_to_read, record_size, &limit_counter);

                if (total_bytes_to_read <= 0 || limit_counter == 0) {
                    read_done = true;
                    break;
                }
            }

            if (read_done) {
                break;
            }
        }
    }
}