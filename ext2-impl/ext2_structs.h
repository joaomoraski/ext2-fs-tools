//
// Created by moraski on 07/06/25.
// Comentario do Clion
//

#ifndef EXT2_STRUCTS_H
#define EXT2_STRUCTS_H
#include <stdint.h>

// packed é pra falar pro compilador para de ser fresco e ler exatamente como esta
// inves de ler em multiplo de 4 adicionando padding ele le exatamente como ta la
typedef struct __attribute__((packed)) {
    unsigned int s_inodes_count;
    unsigned int s_blocks_count;
    unsigned int s_r_blocks_count;
    unsigned int s_free_blocks_count;
    unsigned int s_free_inodes_count;
    unsigned int s_first_data_block;
    unsigned int s_log_block_size;
    unsigned int s_log_frag_size; //s_log_cluster_size frag_size
    unsigned int s_blocks_per_group;
    unsigned int s_frag_per_group; //s_clusters_per_group frag
    unsigned int s_inodes_per_group;
    unsigned int s_mtime;
    unsigned int s_wtime;
    unsigned short s_mnt_count;
    unsigned short s_max_mnt_count;
    unsigned short s_magic;
    unsigned short s_state;
    unsigned short s_errors;
    unsigned short s_minor_rev_level;
    unsigned int s_lastcheck;
    unsigned int s_checkinterval;
    unsigned int s_creator_os;
    unsigned int s_rev_level;
    unsigned short s_def_resuid;
    unsigned short s_def_resgid;
    unsigned int s_first_ino;
    unsigned short s_inode_size;
    unsigned short s_block_group_nr;
    unsigned int s_feature_compat;
    unsigned int s_feature_incompat;
    unsigned int s_feature_ro_compat;
    unsigned char s_uuid[16];
    unsigned char s_volume_name[16];
    unsigned char s_last_mounted[64];
    unsigned int s_algorithm_usage_bitmap;
    unsigned char s_prealloc_blocks;
    unsigned char s_prealloc_dir_blocks;
    unsigned short s_reserved_gdt_blocks;
    unsigned char s_journal_uuid[16];
    unsigned int s_journal_inum;
    unsigned int s_journal_dev;
    unsigned int s_last_orphan;
    unsigned char s_hash_seed[16];
    unsigned char s_def_hash_version;
    unsigned char s_reserved_char_pad[3];
    unsigned int s_default_mount_opts;
    unsigned int s_first_meta_bg;
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


typedef struct __attribute__((packed)) {
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
    uint32_t i_generation; // file version for nfs
    uint32_t i_file_acl; //
    uint32_t i_dir_acl;
    uint32_t i_faddr;
    uint8_t i_osd2[12];
} inode_struct;

typedef struct __attribute__((packed)) {
    uint32_t inode;
    uint16_t rec_len;
    uint8_t name_len;
    uint8_t file_type;
    char  name[255];
} dir_entry;

typedef struct {
    super_block sb;
    group_desc* group_desc_array;
    unsigned int block_size; // Tamanho do bloco em bytes (já calculado de s_log_block_size)
    unsigned int num_block_groups; // Número total de grupos de blocos (calculado do sb)
    unsigned int current_dir_inode; // Inode do diretório corrente (para cd, ls, pwd)
    char current_path[256];
    int fd;
} ext2_info;

#endif //EXT2_STRUCTS_H
