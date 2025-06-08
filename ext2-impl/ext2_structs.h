//
// Created by moraski on 07/06/25.
// Comentario do Clion
//

#ifndef EXT2_STRUCTS_H
#define EXT2_STRUCTS_H
#include <stdint.h>

typedef struct __attribute__((packed)) {
    // unsigned char padding_01[4];
    unsigned int s_inodes_count;
    unsigned int s_blocks_count;
    unsigned int s_r_blocks_count;
    unsigned int s_free_blocks_count;
    unsigned int s_free_inodes_count;
    unsigned int s_first_data_block;
    unsigned int s_log_block_size;
    unsigned int padding_01; //s_log_cluster_size frag_size
    unsigned int s_blocks_per_group;
    unsigned int padding_02; //s_clusters_per_group frag
    unsigned int s_inodes_per_group;
    unsigned char padding_03[12]; // s_mtime
    // unsigned int padding_04; // s_wtime
    // unsigned short padding_05; // s_mnt_count
    // signed short padding_06; // s_max_mnt_count
    unsigned short s_magic;
    unsigned char padding_07[30]; // s_state
    // unsigned short padding_08; // s_errors
    // unsigned short padding_09; // s_minor_rev_level
    // unsigned int padding_10; // s_lastcheck
    // unsigned int padding_11; // s_checkinterval
    // unsigned int padding_12; // s_creator_os
    // unsigned int padding_13; // s_rev_level
    // unsigned short padding_14; // s_def_resuid
    // unsigned short padding_15; // s_def_resgid
    // unsigned int padding_16; // s_first_ino
    unsigned short s_inode_size;
    unsigned char padding_17[30]; // s_block_group_nr
    // unsigned int padding_18; // s_feature_compat
    // unsigned int padding_19; // s_feature_incompat
    // unsigned int padding_20; // s_feature_ro_compat
    // unsigned char padding_21; // s_uuid
    unsigned char s_volume_name[16];
} super_block;

typedef struct __attribute__((packed)) {
    unsigned int bg_block_bitmap;
    unsigned int bg_inode_bitmap;
    unsigned int bg_inode_table;
    unsigned short bg_free_blocks_count;
    unsigned short bg_free_inodes_count;
    unsigned short bg_used_dirs_count;
    unsigned short bg_flags;
    unsigned int bg_exclude_bitmap_lo;
    unsigned short bg_block_bitmap_csum_lo;
    unsigned short bg_inode_bitmap_csum_lo;
    unsigned short bg_itable_unused;
    unsigned short bg_checksum;
} group_desc;


typedef struct {
    uint16_t i_mode;
    uint16_t i_uid;
    uint32_t i_size;
    uint32_t i_atime;
    uint32_t i_ctime;
    uint32_t i_mtime;
    uint32_t i_dtime;
    uint16_t i_gid;
    uint16_t i_links_count;
    uint32_t i_blocks;
    uint32_t i_flags;
    uint32_t i_osd1;
    uint32_t i_block[15];
    uint32_t i_generation;
    uint32_t i_file_acl;
    uint32_t i_dir_acl;
    uint32_t i_faddr;
    uint8_t i_osd2[12];
} inode;

typedef struct {
    super_block sb;
    group_desc* group_desc_array;
    unsigned int block_size; // Tamanho do bloco em bytes (já calculado de s_log_block_size)
    unsigned int num_block_groups; // Número total de grupos de blocos (calculado do sb)
    unsigned int current_dir_inode; // Inode do diretório corrente (para cd, ls, pwd)
    char current_path[256];
} ext2_info;

#endif //EXT2_STRUCTS_H
