//
// Created by moraski on 07/06/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../utils/utils.h"
#include "../ext2-impl/ext2-fs-methods.h"
#include "../ext2-impl/ext2_structs.h"

void info(ext2_info fs_info) {
    super_block super_block = fs_info.sb;
    unsigned int block_size = fs_info.block_size;
    printf("Volume name.....: %s\n"
           "Image size......: %u bytes\n"
           "Free space......: %u KiB\n"
           "Free inodes.....: %u\n"
           "Free blocks.....: %u\n"
           "Block size......: %u bytes\n"
           "Inode size......: %u bytes\n"
           "Groups count....: %u\n"
           "Groups size.....: %u blocks\n"
           "Groups inodes...: %u inodes\n"
           "Inodetable size.: %u blocks\n",
           (char*)super_block.s_volume_name, // funciona sem o cast, mas quis tirar highlight da IDE
           super_block.s_blocks_count * block_size,
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

void ls(ext2_info fs_info) {
    inode_struct inode = read_inode_by_number(&fs_info, fs_info.current_dir_inode);

    char tmp[1024];
    read_data_block(&fs_info, inode.i_block[0], tmp, sizeof(tmp));

    char* actual_pointer = tmp;
    int bytes_read = 0;

    while (bytes_read < BASE_BLOCK) {
        dir_entry* entry = (dir_entry*)actual_pointer;

        if (entry->rec_len == 0) {
            break;
        }

        // 0 significa excluido ou vazio
        if (entry->inode != 0) {
            char name[entry->name_len];
            memcpy(name, entry->name, entry->name_len);
            name[entry->name_len] = '\0';
            printf("%s\n"
                   "inode: %d\n"
                   "record lenght: %d\n"
                   "name lenght: %d\n"
                   "file type: %d\n\n", entry->name, entry->inode, entry->rec_len, entry->name_len, entry->file_type);
        }

        actual_pointer += entry->rec_len;
        bytes_read += entry->rec_len;
    }
}

void print_superblock(ext2_info fs_info) {
    super_block super_block = fs_info.sb;
    char lastcheck[50];
    format_date(super_block.s_lastcheck, lastcheck, 50);
    printf("inodes count: %u\n"
           "blocks count: %u\n"
           "reserved blocks count: %u\n"
           "free blocks count: %u\n"
           "free inodes count: %u\n"
           "first data block: %u\n"
           "block size: %u\n"
           "fragment size: %u\n"
           "blocks per group: %u\n"
           "fragments per group: %u\n"
           "inodes per group: %u\n"
           "mount time: %u\n"
           "write time: %u\n"
           "mount count: %u\n"
           "max mount count: %u\n"
           "magic signature: 0x%x\n"
           "file system state: %u\n"
           "errors: %u\n"
           "minor revision level: %u\n"
           "time of last check: %s\n"
           "max check interval: %u\n"
           "creator OS: %u\n"
           "revision level: %u\n"
           "default uid reserved blocks: %u\n"
           "default gid reserved blocks: %u\n"
           "first non-reserved inode: %u\n"
           "inode size: %u\n"
           "block group number: %u\n"
           "compatible feature set: %u\n"
           "incompatible feature set: %u\n"
           "read only comp feature set: %u\n"
           "UUID: %02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x\n"
           "volume name: %s\n"
           "volume last mounted: %s\n"
           "algorithm usage bitmap: %u\n"
           "blocks to try to preallocate: %d\n"
           "blocks preallocate dir: %u\n"
           "reserved gdt blocks: %u\n"
           "journal uuid: %s\n"
           "journal INum: %u\n"
           "journal Dev: %u\n"
           "last orphan: %u\n"
           "hash seed: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n"
           "default hash version: %u\n"
           "default mount options: %u\n"
           "first meta block group: %u\n",
           super_block.s_inodes_count,
           super_block.s_blocks_count,
           super_block.s_r_blocks_count,
           super_block.s_free_blocks_count,
           super_block.s_free_inodes_count,
           super_block.s_first_data_block,
           1024 << super_block.s_log_block_size,
           super_block.s_log_frag_size,
           super_block.s_blocks_per_group,
           super_block.s_frag_per_group,
           super_block.s_inodes_per_group,
           super_block.s_mtime,
           super_block.s_wtime,
           super_block.s_mnt_count,
           super_block.s_max_mnt_count,
           super_block.s_magic,
           super_block.s_state,
           super_block.s_errors,
           super_block.s_minor_rev_level,
           lastcheck,
           super_block.s_checkinterval,
           super_block.s_creator_os,
           super_block.s_rev_level,
           super_block.s_def_resuid,
           super_block.s_def_resgid,
           super_block.s_first_ino,
           super_block.s_inode_size,
           super_block.s_block_group_nr,
           super_block.s_feature_compat,
           super_block.s_feature_incompat,
           super_block.s_feature_ro_compat,
           super_block.s_uuid[0], super_block.s_uuid[1], super_block.s_uuid[2], super_block.s_uuid[3],
           super_block.s_uuid[4], super_block.s_uuid[5], super_block.s_uuid[6], super_block.s_uuid[7],
           super_block.s_uuid[8], super_block.s_uuid[9], super_block.s_uuid[10], super_block.s_uuid[11],
           super_block.s_uuid[12], super_block.s_uuid[13], super_block.s_uuid[14], super_block.s_uuid[15],
           (char*)super_block.s_volume_name,
           (char*)super_block.s_last_mounted,
           super_block.s_algorithm_usage_bitmap,
           super_block.s_prealloc_blocks,
           super_block.s_prealloc_dir_blocks,
           super_block.s_reserved_gdt_blocks,
           (char*)super_block.s_journal_uuid,
           super_block.s_journal_inum,
           super_block.s_journal_dev,
           super_block.s_last_orphan,
           super_block.s_hash_seed[0], super_block.s_hash_seed[1],
           super_block.s_hash_seed[2], super_block.s_hash_seed[3],
           super_block.s_hash_seed[4], super_block.s_hash_seed[5],
           super_block.s_hash_seed[6], super_block.s_hash_seed[7],
           super_block.s_hash_seed[8], super_block.s_hash_seed[9],
           super_block.s_hash_seed[10], super_block.s_hash_seed[11],
           super_block.s_hash_seed[12], super_block.s_hash_seed[13],
           super_block.s_hash_seed[14], super_block.s_hash_seed[15],
           super_block.s_def_hash_version,
           super_block.s_default_mount_opts,
           super_block.s_first_meta_bg);
}

void print_groups(ext2_info fs_info) {
    for (int i = 0; i < fs_info.num_block_groups; ++i) {
        group_desc group_desc = fs_info.group_desc_array[i];
        printf("Block Group Descriptor %d:\n"
               "block bitmap: %d\n"
               "inode bitmap: %d\n"
               "inode table: %d\n"
               "free blocks count: %d\n"
               "free inodes count: %d\n"
               "used dirs count: %d\n\n", i, group_desc.bg_block_bitmap, group_desc.bg_inode_bitmap,
               group_desc.bg_inode_table, group_desc.bg_free_blocks_count, group_desc.bg_free_inodes_count,
               group_desc.bg_used_dirs_count);
    }
}

void print_inode(ext2_info fs_info, unsigned int inode_number) {
    inode_struct inode = read_inode_by_number(&fs_info, inode_number);
    printf("file format and access rights: 0x%x\n"
           "user id: %d\n"
           "lower 32-bit file size: %d\n"
           "access time: %d\n"
           "creation time: %d\n"
           "modification time: %d\n"
           "deletion time: %d\n"
           "group id: %d\n"
           "link count inode: %d\n"
           "512-bytes blocks: %d\n"
           "ext2 flags: %d\n"
           "reserved (Linux): %d\n", inode.i_mode, inode.i_uid, inode.i_size, inode.i_atime, inode.i_ctime,
           inode.i_mtime, inode.i_dtime, inode.i_gid, inode.i_links_count, inode.i_blocks, inode.i_flags, inode.i_osd1);
    for (int i = 0; i < 15; ++i) {
        printf("pointer[%d]: %d\n", i, inode.i_block[i]);
    }
    printf("file version (nfs): %d\n"
           "block number extended attributes: %d\n"
           "higher 32-bit file size: %d\n"
           "location file fragment: %d\n", inode.i_generation, inode.i_file_acl, inode.i_dir_acl, inode.i_faddr);
}

void cd(ext2_info* fs_info, char* path) {
    char path_copy[1024];
    strcpy(path_copy, path);
    unsigned int inode_number = find_inode_number_by_path(fs_info, path);


    if (inode_number == 0) { // Não achou o diretorio, voltou com o erro
        // Erro ja foi printado
        return;
    }

    inode_struct inode = read_inode_by_number(fs_info, inode_number);

    if (!S_ISDIR(inode.i_mode)) {
        printf("'%s': Não é um diretório\n", path);
        return;
    }

    // atualiza o numero do inode atual na estrutura principal
    fs_info->current_dir_inode = inode_number;


    // atualizar a string do caminho

    if (strcmp(path, "..") == 0) {
        // se o comando foi 'cd ..'.
        change_path_level(fs_info->current_path);
    } else if (strcmp(path, "/") == 0) {
        // Se o comando for 'cd /', reseta o caminho para a raiz.
        strcpy(fs_info->current_path, "/");
    } else if (path[0] == '/') {
        // se for um caminho absoluto, o novo caminho é o próprio path.
        strcpy(fs_info->current_path, path);
    } else {
        // se for um caminho relativo, anexe ao caminho atual.
        if (strcmp(fs_info->current_path, "/") != 0) {
            strcat(fs_info->current_path, "/");
        }
        strcat(fs_info->current_path, path);
    }
}
