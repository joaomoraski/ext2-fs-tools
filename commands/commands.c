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

void print_data_block(ext2_info* fs_info, unsigned int block_number, char* block_buffer, long* total_length,
                      long* bytes_read);

// função para o comando info
// printa as principais informações do superbloco
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
           (super_block.s_free_blocks_count - super_block.s_r_blocks_count) * block_size / 1024,
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

// função do comando ls
// lista baseado no diretorio passado ou no diretorio atual
void ls(ext2_info fs_info, char* path) {
    inode_struct inode;
    // verificação se o ls passa um caminho ou não
    if (path != NULL) {
        char tmp_path[1024];
        strcpy(tmp_path, path);
        // acha o inode pelo path informado
        unsigned int inode_number = find_inode_number_by_path(&fs_info, tmp_path);

        if (inode_number == 0) {
            printf("ls: falha ao encontrar o diretorio '%s'\n", path);
            return;
        }

        // dps de pegar o inode_number, pega o inode desse numero
        inode = read_inode_by_number(&fs_info, inode_number);

        // se não for diretorio(tentou listar um arquivo) ele so printa o arquivo(testado no linux)
        if (!is_dir(inode.i_mode)) {
            printf("%s\n", path);
            return;
        }
    } else {
        // se n tem path, so carrega pelo inode do diretorio atual
        inode = read_inode_by_number(&fs_info, fs_info.current_dir_inode);
    }


    // le o datablock do indicado pelo inode
    // diretorios so tem 1 bloco
    char tmp[1024];
    read_data_block(&fs_info, inode.i_block[0], tmp, sizeof(tmp));

    // faz o loop pelo datablock printando cada entrada de diretorio
    char* actual_pointer = tmp;
    int bytes_read = 0;

    while (bytes_read < BASE_BLOCK) {
        // pega o ponteiro atual e passa para ser o inicio do entry
        dir_entry* entry = (dir_entry*)actual_pointer;

        // se rec_len == 0 então algo esta errado
        if (entry->rec_len == 0) break;

        // 0 significa excluido ou vazio
        if (entry->inode != 0) {
            // cria o name pelo tamanho do nome da entry
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

// print superblock, usado para depuração
// printa todas as informações do superbloco
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


// print groups, usado para depuração
// printa todas as informações dos descritores de grupo
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

// print inode, usado para depuração
// printa todas as informações do inode informado
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

// comando cd
// faz a troca de diretorio, atualiza o path e o current_dir_inode
void cd(ext2_info* fs_info, char* path) {
    // faz uma copia do caminho para a manipulação dele não acabar corrompendo o original
    char path_copy[1024];
    strcpy(path_copy, path);
    // encontra o numero do inode pelo caminho informado
    unsigned int inode_number = find_inode_number_by_path(fs_info, path_copy);

    // se for 0, informa que tem algo errado
    if (inode_number == 0) {
        // Não achou o diretorio, voltou com o erro
        printf("cd: o diretorio '%s': não foi encontrado.\n", path);
        return;
    }

    // pega o inode pelo numero informado
    inode_struct inode = read_inode_by_number(fs_info, inode_number);

    // verifica se é diretorio, se não for, retorna erro
    if (!is_dir(inode.i_mode)) {
        printf("'%s': Não é um diretório\n", path);
        return;
    }
    // atualiza o numero do inode atual na estrutura principal
    fs_info->current_dir_inode = inode_number;

    // atualizar a string do caminho
    char new_path[1024];
    resolve_path_string(new_path, fs_info->current_path, path);
    // copia o caminho novo para o atual, substituindo o valor
    strcpy(fs_info->current_path, new_path);
}

// comando attr
// retorna informações como o tamanho, permissoes, datas de modificação e etc
void attr(ext2_info* fs_info, char* path) {
    // faz uma copia do caminho para não mudar o informado
    char path_copy[1024];
    strcpy(path_copy, path);
    // pega o numero do inode pelo path informado
    unsigned int inode_number = find_inode_number_by_path(fs_info, path);

    // se não achou, informa erro
    if (inode_number == 0) {
        // Não achou o diretorio, voltou com o erro
        printf("attr: o arquivo '%s': não foi encontrado.\n", path);
        return;
    }

    // carrega o inode pelo numero
    inode_struct inode = read_inode_by_number(fs_info, inode_number);
    char permissions_string[100];
    // monta a string de permissões baseado no i_mode
    mount_permissions_string(inode.i_mode, permissions_string);
    char buffer[1024];
    // formata a data para o formato de exemplo
    format_date(inode.i_atime, buffer, 1024);
    // printa as informações
    printf("permissões\tuid\tgid\ttamanho\tmodificado em\n"
           "%s\t%hu\t%hu\t%.1f KiB\t%s\n", permissions_string, inode.i_uid, inode.i_gid, inode.i_size / 1024.0, buffer);
}

// comando cat
// faz a leitura dos blocos de um arquivo
void cat(ext2_info* fs_info, char* path) {
    // faz uma copia do caminho para não mudar o informado
    char path_copy[1024];
    strcpy(path_copy, path);
    // pega o numero do inode pelo path informado
    unsigned int inode_number = find_inode_number_by_path(fs_info, path);

    // se não achou, informa erro
    if (inode_number == 0) {
        // Não achou o diretorio, voltou com o erro
        printf("cat: o arquivo '%s': não foi encontrado.\n", path);
        return;
    }

    // carrega o inode pelo numero dele
    inode_struct inode = read_inode_by_number(fs_info, inode_number);
    // se for diretorio, informa um erro
    if (is_dir(inode.i_mode)) {
        printf("cat: '%s': É um diretório\n", path); // roubei o padrao do linux
        return;
    }

    // calcula o tamanho total e o numero de dados lidos
    long total_length = inode.i_size;
    long bytes_read = 0;

    // block buffer do tamanho do bloco
    char block_buffer[fs_info->block_size];

    // variavel de controle de leitura
    bool read_done = false;

    // apenas lidando com 12 entradas
    // 13 e 14 sao ponteiros indiretos
    for (int i = 0; i < 12; ++i) {
        // pega o numero do bloco na posição i
        unsigned int block_number = inode.i_block[i];

        // previnir de printar lixo de memoria
        // não tem mais blocos de dados, ou esta com algum problema
        if (block_number == 0) continue;

        // função para printar as informações do bloco de dados
        print_data_block(fs_info, block_number, block_buffer, &total_length, &bytes_read);

        // se o total de lidos for maior ou igual ao tamanho total, sai do loop
        // e seta a variavel para indicar que ja acabou de ler
        if (bytes_read >= total_length) {
            read_done = true;
            break;
        }
    }

    // bloco indireto
    // contem uma lista de ponteiros para outros blocos de dados
    // inode -> bloco de ponteiros -> bloco de dados
    if (!read_done && inode.i_block[12] != 0) {
        // Este bloco contém uma lista de ponteiros para blocos de dados.
        // 256 ponteiros cada pointer tem 4 bytes, 1024/4 = 256
        unsigned int pointers_block[256];
        read_data_block(fs_info, inode.i_block[12], (char*)pointers_block, fs_info->block_size);

        // loop pelos ponteiros lidos
        for (int i = 0; i < 256; i++) {
            unsigned int block_number = pointers_block[i];
            if (block_number == 0) continue; // ignora este bloco

            print_data_block(fs_info, block_number, block_buffer, &total_length, &bytes_read);

            // seta a variavel para indicar que ja acabou de ler
            if (bytes_read >= total_length) {
                read_done = true;
                break;
            }
        }
    }

    // bloco indireto duplo
    // aponta para um bloco que contem a lista de ponteiros
    // e cada um desses ponteiros aponta para outro bloco
    // inode -> bloco de ponteiros 1 -> bloco de ponteiros 2 -> bloco de dados
    if (!read_done && inode.i_block[13] != 0) {
        // ponteiros level 1
        unsigned int lv1_pointers[256];
        read_data_block(fs_info, inode.i_block[13], (char*)lv1_pointers, fs_info->block_size);

        // loop nos ponteiros
        for (int i = 0; i < 256; i++) {
            if (lv1_pointers[i] == 0) continue;

            // le o segundo bloco de ponteiros
            // ponteiros de lv 2
            unsigned int lv2_pointers[256];
            read_data_block(fs_info, lv1_pointers[i], (char*)lv2_pointers, fs_info->block_size);

            // loop no segundo level de ponteiros, que apontam para dados
            for (int j = 0; j < 256; j++) {
                unsigned int block_number = lv2_pointers[j];
                if (block_number == 0) continue;

                print_data_block(fs_info, block_number, block_buffer, &total_length, &bytes_read);

                // seta a variavel para indicar que ja acabou de ler
                if (bytes_read >= total_length) {
                    read_done = true;
                    break;
                }
            }

            // se terminou de ler sai do for de cima
            if (read_done) {
                break;
            }
        }
    }

    // nao é necessario o i_block[14] por conta do que a doc fala:
    //      https://www.nongnu.org/ext2-doc/ext2.html#i-block:~:text=The%2014th%20entry,doubly%2Dindirect%20block.
    // O ponteiro indireto triplo (i_block[14]) seria a mesma lógica com mais um loop aninhado,
    // mas para arquivos de até 64MB, o indireto duplo já é suficiente.
    printf("\n");
    fflush(stdout); // garante a impressão total na tela antes de seguir
}

void write_data_block_out(ext2_info* fs_info, unsigned int block_number, char block_buffer[], long total_length,
                          long* bytes_read, FILE* target_file) {
    // le o data block
    read_data_block(fs_info, block_number, block_buffer, fs_info->block_size);
    // evitar copiar lixo de memoria
    // calcular quantos bytes ainda faltam para ser copiado
    long remaining_bytes = total_length - *bytes_read;

    // decide quantos bytes escrever do bloco atual
    int bytes_to_write = (remaining_bytes < fs_info->block_size) ? remaining_bytes : fs_info->block_size;
    // se o que falta é menor que o bloco, escreve so o que falta
    // se não, escreve o bloco inteiro

    // usar fwrite por que ele imprime os dados brutos(binarios)
    // não para em \0
    fwrite(block_buffer, 1, bytes_to_write, target_file);
    *bytes_read += bytes_to_write;
}

// função para facilitar a impressão na tela, ja repetida em 3 lugares diferentes
void print_data_block(ext2_info* fs_info, unsigned int block_number, char* block_buffer, long* total_length,
                      long* bytes_read) {
    // le o data block
    read_data_block(fs_info, block_number, block_buffer, fs_info->block_size);
    // evitar printar lixo de memoria
    // calcular quantos bytes ainda faltam para ser lido o arquivo todo
    long remaining_bytes = total_length - bytes_read;

    // decide quantos bytes imprimir do bloco atual
    int bytes_to_print = (remaining_bytes < fs_info->block_size) ? remaining_bytes : fs_info->block_size;
    // se o que falta é menor que o bloco, imprime so o que falta
    // se não, imprime o bloco inteiro

    // usar fwrite por que ele imprime os dados brutos(binarios)
    // não para em \0
    fwrite(block_buffer, 1, bytes_to_print, stdout);
    bytes_read += bytes_to_print;
}

// comando touch
// cria arquivos, aceita path tbm para criar dentro de pastas
void touch(ext2_info* fs_info, char* path_to_file) {
    // faz uma copia do caminho para não corromper
    char path_copy[1024];
    strcpy(path_copy, path_to_file);
    // string para separar o nome do arquivo
    char filename[256];
    // pega o numero do inode pai e o nome do arquivo
    unsigned int parent_inode_number = find_parent_inode_and_filename(fs_info, path_copy, filename);
    // se não encontrou, informa erro
    if (parent_inode_number == 0) {
        printf("touch: diretorio pai não encontrado!\n"); // todo pegar o padrao do linux
        return;
    }

    // le o inode do pai
    inode_struct parent_inode = read_inode_by_number(fs_info, parent_inode_number);
    // verifica se o inode é um diretorio, se não for informa que parte do caminho não é um diretorio
    if (!is_dir(parent_inode.i_mode)) {
        printf("touch: parte do caminho não é um diretorio\n"); // todo pegar o padrao do linux
        return;
    }

    // percorrer o data block verificando se o arquivo existe
    if (verify_file_exists(fs_info, parent_inode.i_block[0], filename)) {
        printf("touch: arquivo '%s' já existe\n", filename); // todo pegar o padrao do linux
        return;
    }

    // verificar se o bloco possui espaço para a alocação de novas coisas
    // modo dry-run do add_dir_entry para verificar isso
    bool has_space = add_dir_entry(fs_info, parent_inode_number, 0, filename, 0, false);
    if (!has_space) {
        // print de erro ja esta la dentro
        return;
    }

    // aloca um novo inode e pega o numero dele
    unsigned int new_inode_num = allocate_item(fs_info, 'i');
    if (new_inode_num == 0) {
        // ocorreu um erro
        // print ja esta dentro da função
        return;
    }

    // seta tudo com 0 inicialmente
    // como é touch, garante que o i_block[0-14] é preenchido com 0
    inode_struct new_inode = {0};
    // as mascaras de permissao foram tiradas da lib do ext2
    new_inode.i_mode = EXT2_S_IFREG | EXT2_S_IRUSR | EXT2_S_IWUSR | EXT2_S_IRGRP | EXT2_S_IROTH;; // frw-r--r--
    new_inode.i_size = 0;
    new_inode.i_blocks = 0;
    new_inode.i_links_count = 1;
    time_t timestamp = time(NULL);
    new_inode.i_ctime = timestamp;
    new_inode.i_mtime = timestamp;
    new_inode.i_atime = timestamp;

    // escreve as informações dos inodes pelo numero novo informado
    write_inode_by_number(fs_info, new_inode_num, &new_inode);

    // adiciona a nova entrada de diretorio para o arquivo novo
    add_dir_entry(fs_info, parent_inode_number, new_inode_num, filename, EXT2_FT_REG_FILE, true);
}

// comando mkdir
// executa a criação de pastas
void cmd_mkdir(ext2_info* fs_info, char* path_to_file) {
    // faz a copia d path para não corromper o caminho
    char path_copy[1024];
    strcpy(path_copy, path_to_file);
    // pega o novo nome de diretorio
    char new_dir_name[256];

    // pega o numero do inode pai
    unsigned int parent_inode_number = find_parent_inode_and_filename(fs_info, path_to_file, new_dir_name);
    // se o diretorio pai não existe informa erro
    if (parent_inode_number == 0) {
        printf("mkdir: erro, diretorio pai não existe\n");
        return;
    }

    // carrega o inode pai pelo numero informado
    inode_struct parent_inode = read_inode_by_number(fs_info, parent_inode_number);
    // verifica se é um diretorio ou não
    if (!is_dir(parent_inode.i_mode)) {
        printf("mkdir: parte do caminho não é diretorio\n");
        return;
    }

    // verifica se o diretorio ja existe
    if (verify_file_exists(fs_info, parent_inode.i_block[0], new_dir_name)) {
        printf("mkdir: não foi possível criar o diretório \"%s\": Arquivo existe\n", path_to_file);
        return;
    }

    // verificar se o bloco possui espaço para a alocação de novas coisas
    bool has_space = add_dir_entry(fs_info, parent_inode_number, 0, new_dir_name,
        0, false);
    if (!has_space) {
        // print de erro ja esta la dentro
        return;
    }

    // cria o inode do diretorio, para indicar para um datablock onde vai ter os dados do diretorio
    unsigned int new_dir_inode_num = allocate_item(fs_info, 'i');
    // caso de erro ja retorna
    if (new_dir_inode_num == 0) return;
    // aloca o bloco para o novo diretorio
    unsigned int new_data_block_num = allocate_item(fs_info, 'b');
    // caso de erro, tem que desalocar o inode ja criado
    if (new_data_block_num == 0) {
        // desaloca o inode ja criado
        deallocate_item(fs_info, new_dir_inode_num, 'i');
        return;
    }

    // pegar o grupo do inode atual
    // mesma logica do -1 para garantir o arredondamento
    int group_of_new_inode = (new_dir_inode_num - 1) / fs_info->sb.s_inodes_per_group;

    // incrementar o contador de diretorios daquele grupo
    fs_info->group_desc_array[group_of_new_inode].bg_used_dirs_count++;

    // pegar a posiçao do group desc na imagem
    unsigned int group_desc_position = fs_info->block_size * 2 + (group_of_new_inode * sizeof(group_desc));
    // salva a alteraçao do used_dirs_count
    point_and_write(fs_info->fd, group_desc_position, SEEK_SET, &fs_info->group_desc_array[group_of_new_inode],
                    sizeof(group_desc));

    // aumenta a contagem de links do pai e ja salva
    parent_inode.i_links_count++;
    write_inode_by_number(fs_info, parent_inode_number, &parent_inode);

    // monta o inode novo
    // seta tudo com 0 inicialmente
    // como é touch, garante que o i_block[0-14] é preenchido com 0
    inode_struct new_inode = {0};
    // as mascaras de permissao foram tiradas da lib do ext2
    new_inode.i_mode = EXT2_S_IFDIR | // d
        EXT2_S_IRUSR | EXT2_S_IWUSR | EXT2_S_IXUSR | // dono rwx
        EXT2_S_IRGRP | EXT2_S_IXGRP | // grupo r-x
        EXT2_S_IROTH | EXT2_S_IXOTH;; // outros r-x | fim => drw-r--r--
    new_inode.i_size = fs_info->block_size; // ocupa 1 bloco
    new_inode.i_block[0] = new_data_block_num;
    new_inode.i_links_count = 2; // começa com 2 por conta do . e ..
    new_inode.i_blocks = fs_info->block_size / 512; // quantos setores ele ocupa(como 1 diretorio == 1024)
    time_t timestamp = time(NULL);
    new_inode.i_ctime = timestamp;
    new_inode.i_mtime = timestamp;
    new_inode.i_atime = timestamp;

    // escreve na posição deste inode as informações do novo inode
    write_inode_by_number(fs_info, new_dir_inode_num, &new_inode);

    // cria o buffer de bloco e seta com 0
    // para previnir lixo de memoria de nomes e etc
    char block_buffer[fs_info->block_size];
    // feito apos descobrir erro em criar um diretorio com nome maior que o anterior deletado
    memset(block_buffer, 0, fs_info->block_size);

    // cria as entradas de diretorio . e .. no buffer do diretorio
    dir_entry* self_entry = (dir_entry*)block_buffer;
    self_entry->inode = new_dir_inode_num;
    self_entry->name_len = 1;
    self_entry->file_type = EXT2_FT_DIR;
    strcpy(self_entry->name, ".");
    // 8 bytes da estrutura fixa, 1 de tamanho do nome + 3 de padding ?
    self_entry->rec_len = (8 + self_entry->name_len + 3) & ~3; // 12

    // move o ponteiro para dps da primeira entrada
    char* pointer = block_buffer + self_entry->rec_len;
    dir_entry* parent_entry = (dir_entry*)pointer;
    parent_entry->inode = parent_inode_number;
    parent_entry->name_len = 2;
    parent_entry->file_type = EXT2_FT_DIR;
    strcpy(parent_entry->name, "..");
    // como é o ultimo, mesma logica do inode
    // aloca o tamanho total restante
    parent_entry->rec_len = fs_info->block_size - self_entry->rec_len;

    // salva a informações no novo bloco(do diretorio)
    point_and_write(fs_info->fd, new_data_block_num * fs_info->block_size, SEEK_SET,
                    block_buffer, fs_info->block_size);

    // adiciona o novo diretorio no diretorio pai
    add_dir_entry(fs_info, parent_inode_number, new_dir_inode_num, new_dir_name,
                  EXT2_FT_DIR, true);
}

// comando rm, remove arquivos informados pelo path
void rm(ext2_info* fs_info, char* path) {
    char filename[1024];
    // faz a copia do path
    char path_copy[1024];
    strcpy(path_copy, path);
    // pega o inode do arquivo desejado a ser removido
    int target_inode_number = find_inode_number_by_path(fs_info, path_copy);

    // se não foi encontrado retorna erro
    if (target_inode_number == 0) {
        // Não achou o diretorio, voltou com o erro
        printf("rm: o arquivo '%s': não foi encontrado.\n", path);
        return;
    }

    // refaz a copia do path
    strcpy(path_copy, path);
    // pega o numero do inode pai e separa o filename
    int parent_inode_number = find_parent_inode_and_filename(fs_info, path_copy, filename);

    // esse erro não seria tao necessário já que o find_inode_number_by_path já retornaria erro, mas achei melhor tratar
    // se não achar o pai print erro
    if (parent_inode_number == 0) {
        // não achou o diretorio, voltou com o erro
        printf("rm: o diretorio pai não foi encontrado.\n");
        return;
    }

    // le o inode do arquivo a ser removido
    inode_struct target_inode = read_inode_by_number(fs_info, target_inode_number);
    // se for diretorio informa erro
    if (is_dir(target_inode.i_mode)) {
        printf("rm: erro \"%s\" é um diretorio\n", filename);
        return;
    }

    // diminui a contagem de link do proprio arquivo
    target_inode.i_links_count--;
    // seta o time de delete para o atual
    target_inode.i_dtime = time(NULL);

    // escreve esta mudança na memoria do inode
    write_inode_by_number(fs_info, target_inode_number, &target_inode);

    // remove a entrada do diretorio para deixar o inode orfao
    remove_dir_entry(fs_info, parent_inode_number, filename);

    // apaga os blocos normais
    for (int i = 0; i < 12; ++i) {
        if (target_inode.i_block[i] != 0) {
            // apaga bloco por bloco
            deallocate_item(fs_info, target_inode.i_block[i], 'b');
        }
    }

    // bloco indireto
    // contem uma lista de ponteiros para outros blocos de dados
    // inode -> bloco de ponteiros -> bloco de dados
    if (target_inode.i_block[12] != 0) {
        // ler os blocos que tem uma lista de ponteiros
        unsigned int pointers_block[256];
        read_data_block(fs_info, target_inode.i_block[12], (char*)pointers_block, fs_info->block_size);

        // apagar cada bloco registrado
        for (int i = 0; i < 256; i++) {
            if (pointers_block[i] != 0) {
                // apaga bloco por bloco
                deallocate_item(fs_info, pointers_block[i], 'b');
            }
        }
        // apaga bloco por bloco
        deallocate_item(fs_info, target_inode.i_block[12], 'b');
    }

    // bloco indireto duplo
    // aponta para um bloco que contem a lista de ponteiros
    // e cada um desses ponteiros aponta para outro bloco
    // inode -> bloco de ponteiros 1(1 bloco ocm varios ponteiros) -> bloco de ponteiros 2 -> bloco de dados
    if (target_inode.i_block[13] != 0) {
        // ler os blocos de lv 1
        unsigned int lv1_pointers[256];
        read_data_block(fs_info, target_inode.i_block[13], (char*)lv1_pointers, fs_info->block_size);

        for (int i = 0; i < 256; i++) {
            if (lv1_pointers[i] == 0) continue;
            // percorrer o lv1 lendo o lv2
            unsigned int lv2_pointers[256];
            read_data_block(fs_info, lv1_pointers[i], (char*)lv2_pointers, fs_info->block_size);
            for (int j = 0; j < 256; j++) {
                unsigned int block_number = lv2_pointers[j];
                if (block_number == 0) continue;
                deallocate_item(fs_info, block_number, 'b');
            }
            deallocate_item(fs_info, lv1_pointers[i], 'b');
        }

        deallocate_item(fs_info, target_inode.i_block[13], 'b');
    }

    // 14 nao é necessario porque o 13 ja cobre os 64mb
    deallocate_item(fs_info, target_inode_number, 'i');
}

// comando rmdir, remove diretorios pelo path informado
void cmd_rmdir(ext2_info* fs_info, char* path) {
    char filename[1024];
    // faz a copia do path
    char path_copy[1024];
    strcpy(path_copy, path);

    // localiza o inode do diretorio a ser removido
    int target_inode_number = find_inode_number_by_path(fs_info, path_copy);

    // se não achou, retorna erro
    if (target_inode_number == 0) {
        // Não achou o diretorio, voltou com o erro
        printf("rmdir: o diretorio '%s': não foi encontrado.\n", path);
        return;
    }

    // refaz a copia
    strcpy(path_copy, path);
    // pega o inode do pai e preenche o filename
    int parent_inode_number = find_parent_inode_and_filename(fs_info, path_copy, filename);

    // se não achou o pai, retorna erro
    // esse erro não seria tao necessário já que o find_inode_number_by_path já retornaria erro, mas achei melhor tratar
    if (parent_inode_number == 0) {
        // Não achou o diretorio, voltou com o erro
        printf("rmdir: o diretorio pai '%s': não foi encontrado.\n", path);
        return;
    }

    // se for . ou .. não pode permitir apagar
    if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0) {
        printf("rm: erro \"%s\" não é permitido a remoção deste diretorio\n", path);
        return;
    }

    // pega o inode a ser removido
    inode_struct target_inode = read_inode_by_number(fs_info, target_inode_number);
    // se não for diretorio informa erro
    if (!is_dir(target_inode.i_mode)) {
        printf("rmdir: falhou em remover '%s': Não é um diretorio\n", filename);
        return;
    }

    // le o data block do diretorio
    char tmp[1024];
    read_data_block(fs_info, target_inode.i_block[0], tmp, sizeof(tmp));

    char* actual_pointer = tmp;
    int bytes_read = 0;

    // percorre as entradas de diretorio do alvo para ver se existe alguma coisa dentro
    while (bytes_read < BASE_BLOCK) {
        dir_entry* entry = (dir_entry*)actual_pointer;

        if (entry->rec_len == 0) break;

        // 0 significa excluido ou vazio
        // se tiver . ou .. ele desconsidera
        if (entry->inode != 0 && strcmp(entry->name, ".") != 0 && strcmp(entry->name, "..") != 0) {
            printf("rmdir: falhou em remover '%s': Diretório não está vazio\n", filename);
            return;
        }

        // atualiza as contagens
        actual_pointer += entry->rec_len;
        bytes_read += entry->rec_len;
    }

    // decrementar o contador de links do diretorio do pai
    inode_struct parent_inode = read_inode_by_number(fs_info, parent_inode_number);

    // diminui a contagem de links do pai
    parent_inode.i_links_count--;
    // seta a do target para 0 e a data de delete
    target_inode.i_links_count = 0;
    target_inode.i_dtime = time(NULL);

    // pega o numero do grupo que o inode esta
    int group_to_update = (target_inode_number - 1) / fs_info->sb.s_inodes_per_group;
    // atualiza a informação de dirs count
    fs_info->group_desc_array[group_to_update].bg_used_dirs_count--;
    // escreve as informações do pai e do inode alvo
    write_inode_by_number(fs_info, parent_inode_number, &parent_inode);
    write_inode_by_number(fs_info, target_inode_number, &target_inode);
    // pega a posição do group desc do grupo alterado
    unsigned int group_desc_position = fs_info->block_size * 2 + (group_to_update * sizeof(group_desc));
    // altera os dados do grupo
    point_and_write(fs_info->fd, group_desc_position, SEEK_SET, &fs_info->group_desc_array[group_to_update],
                    sizeof(group_desc));

    // remover o dir_entry do diretorio
    remove_dir_entry(fs_info, parent_inode_number, filename);

    // desalocar o bloco 0(especificaçao diz que o diretorio ocupa apenas 1 bloco)
    deallocate_item(fs_info, target_inode.i_block[0], 'b');
    // desalocaçao do inode do diretorio
    deallocate_item(fs_info, target_inode_number, 'i');
}

// comando cp, faz a copia de um arquivo de dentro da imagem para fora
int cp(ext2_info* fs_info, char* source_path, char* target_path) {
    // abre o arquivo
    // write binary para ser mais facil a transição de dados
    FILE* target_file = fopen(target_path, "wb");
    // se der erro, pode ser permissoa, nao existe e etc
    if (target_file == NULL) {
        printf("cp: erro na abertura do arquivo no sistema\n");
        return EXIT_FAILURE;
    }
    // faz a copia do path
    char copy_path[1024];
    strcpy(copy_path, source_path);
    // pega o numero do inode pelo caminho informado
    unsigned int inode_number = find_inode_number_by_path(fs_info, copy_path);

    if (inode_number == 0) {
        printf("cp: erro, arquivo '%s', não existe", source_path);
        return EXIT_FAILURE;
    }

    // carrega o inode pelo numero passado
    inode_struct inode = read_inode_by_number(fs_info, inode_number);

    // se for diretorio informa erro
    if (is_dir(inode.i_mode)) {
        printf("cp: erro '%s' é um diretorio\n", source_path);
        return EXIT_FAILURE;
    }

    // pega o tamanho total pelo tamanho do inode
    long total_length = inode.i_size;
    long bytes_read = 0;

    // cria o buffer pelo tamanho do bloco
    char block_buffer[fs_info->block_size];

    bool read_done = false;

    // apenas lidando com 12 entradas
    // 13 e 14 sao ponteiros indiretos
    for (int i = 0; i < 12; ++i) {
        unsigned int block_number = inode.i_block[i];

        // previnir de copiar lixo de memoria
        // não tem mais blocos de dados
        if (block_number == 0) {
            break;
        }

        // escreve os dados para fora
        write_data_block_out(fs_info, block_number, block_buffer, total_length, &bytes_read, target_file);

        // se o total de lidos for maior ou igual ao tamanho total, sai do loop
        // e seta a variavel para indicar que ja acabou de ler
        if (bytes_read >= total_length) {
            read_done = true;
            break;
        }
    }

    // bloco indireto
    // contem uma lista de ponteiros para outros blocos de dados
    // inode -> bloco de ponteiros -> bloco de dados
    if (!read_done && inode.i_block[12] != 0) {
        // Este bloco contém uma lista de ponteiros para blocos de dados.
        // 256 ponteiros cada pointer tem 4 bytes, 1024/4 = 256
        unsigned int pointers_block[256];
        read_data_block(fs_info, inode.i_block[12], (char*)pointers_block, fs_info->block_size);

        // loop pelos ponteiros lidos
        for (int i = 0; i < 256; i++) {
            unsigned int block_number = pointers_block[i];
            if (block_number == 0) continue; // ignora este bloco

            write_data_block_out(fs_info, block_number, block_buffer, total_length, &bytes_read, target_file);

            // seta a variavel para indicar que ja acabou de ler
            if (bytes_read >= total_length) {
                read_done = true;
                break;
            }
        }
    }

    // bloco indireto duplo
    // aponta para um bloco que contem a lista de ponteiros
    // e cada um desses ponteiros aponta para outro bloco
    // inode -> bloco de ponteiros 1 -> bloco de ponteiros 2 -> bloco de dados
    if (!read_done && inode.i_block[13] != 0) {
        // ponteiros level 1
        unsigned int lv1_pointers[256];
        read_data_block(fs_info, inode.i_block[13], (char*)lv1_pointers, fs_info->block_size);

        // loop nos ponteiros
        for (int i = 0; i < 256; i++) {
            if (lv1_pointers[i] == 0) continue;

            // le o segundo bloco de ponteiros
            // ponteiros de lv 2
            unsigned int lv2_pointers[256];
            read_data_block(fs_info, lv1_pointers[i], (char*)lv2_pointers, fs_info->block_size);

            // loop no segundo level de ponteiros, que apontam para dados
            for (int j = 0; j < 256; j++) {
                unsigned int block_number = lv2_pointers[j];
                if (block_number == 0) continue;

                write_data_block_out(fs_info, block_number, block_buffer, total_length, &bytes_read, target_file);

                // seta a variavel para indicar que ja acabou de ler
                if (bytes_read >= total_length) {
                    read_done = true;
                    break;
                }
            }

            // se terminou de ler sai do for de cima
            if (read_done) break;
        }
    }
    fclose(target_file);
    return EXIT_SUCCESS;
}


// comando mv, move um arquivo para fora da imagem
// implementação basica, fiz o cp com um rm em caso se sucesso
void mv(ext2_info* fs_info, char* source_path, char* target_path) {
    // cp ja faz o trabalho de copiar o arquivo para fora
    int result = cp(fs_info, source_path, target_path);


    if (result == EXIT_SUCCESS) {
        rm(fs_info, source_path);
    }
}

// cmd rename
// renomea o arquivo informado
void cmd_rename(ext2_info* fs_info, char* source_name, char* new_name) {
    // faz a copia do path
    char source_path[1024];
    strcpy(source_path, source_name);
    // pega o arquivo pelo path informado
    unsigned int inode_number = find_inode_number_by_path(fs_info, source_path);

    // se for 0 ele informa que n existe
    if (inode_number == 0) {
        printf("rename: erro '%s' não existe\n", source_name);
        return;
    }

    // faz a copia do source path
    strcpy(source_path, source_name);
    // pega o filename atual do arquivo
    char filename[1024];
    unsigned int parent_inode_number = find_parent_inode_and_filename(fs_info, source_path, filename);

    // se o pai não foi encontrado retorna erro
    if (parent_inode_number == 0) {
        // Não achou o diretorio, voltou com o erro
        printf("rename: o diretorio pai: não foi encontrado.\n");
        return;
    }

    // carrega o inode do pai e o do alvo
    inode_struct parent_inode = read_inode_by_number(fs_info, parent_inode_number);
    inode_struct target_inode = read_inode_by_number(fs_info, inode_number);

    // se existe informa erro
    if (verify_file_exists(fs_info, parent_inode.i_block[0], new_name)) {
        printf("rename: falhou em renomear para '%s': Arquivo já existe\n", new_name);
        return;
    }

    // remove a entrada anterior
    if (!remove_dir_entry(fs_info, parent_inode_number, source_name)) {
        printf("rename: falha ao remover a entrada de diretório antiga.\n");
        return;
    }

    // controla o tipo da nova dir_entry pelo tipo do inode atual
    int file_type = is_dir(target_inode.i_mode) ? EXT2_FT_DIR : EXT2_FT_REG_FILE;
    // adiciona a nova entrada, usando o mesmo inode, mas nome diferente.
    if (!add_dir_entry(fs_info, parent_inode_number, inode_number, new_name, file_type, true)) {
        printf("rename: erro ao recriar entrada de diretório. O sistema pode estar inconsistente.\n");
    }
}

// implementação do comando touch para varios arquivos
// não sei se é o correto, mas fiz apenas um loop dos valores
void multi_touch(ext2_info* fs_info, char** args, int argc) {
    for (int i = 1; i < argc; ++i) {
        touch(fs_info, args[i]);
    }
}

// implementação do comando mkdir para varios arquivos
// não sei se é o correto, mas fiz apenas um loop dos valores
void multi_cmd_mkdir(ext2_info* fs_info, char** args, int argc) {
    for (int i = 1; i < argc; ++i) {
        cmd_mkdir(fs_info, args[i]);
    }
}

// implementação do comando rm para varios arquivos
// não sei se é o correto, mas fiz apenas um loop dos valores
void multi_rm(ext2_info* fs_info, char** args, int argc) {
    for (int i = 1; i < argc; ++i) {
        rm(fs_info, args[i]);
    }
}

// implementação do comando rmdir para varios arquivos
// não sei se é o correto, mas fiz apenas um loop dos valores
void multi_cmd_rmdir(ext2_info* fs_info, char** args, int argc) {
    for (int i = 1; i < argc; ++i) {
        cmd_rmdir(fs_info, args[i]);
    }
}
