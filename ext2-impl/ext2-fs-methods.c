//
// Created by moraski on 07/06/25.
//
#include "ext2-fs-methods.h"

#include <stdbool.h>
#include <string.h>

#include "../utils/utils.h"

// carrega as informações do superbloco
void load_super_block(ext2_info* fs_info) {
    // inicializa a struct do super_block
    super_block super_block;

    // se movimenta ate o offset do base_block e le o conteudo para a struct do super_block
    lseek(fs_info->fd, BASE_BLOCK, SEEK_SET);
    read(fs_info->fd, &super_block, sizeof(super_block));

    // verificação para se é ou não ext2 filesystem
    if (super_block.s_magic != EXT2_SUPER_MAGIC) {
        fprintf(stderr, "Not an ext2 filesystem\n");
        exit(EXIT_FAILURE);
    }

    // preenche o superblock da struct principal do sistema
    fs_info->sb = super_block;
    // shift bit a bit para calcular o tamanho do bloco
    // define o valor baseado no valor logaritmo cadastrado no superbloco
    // igual definido na doc
    //      https://www.nongnu.org/ext2-doc/ext2.html#s-log-block-size
    fs_info->block_size = 1024 << super_block.s_log_block_size;
}

// carrega as informações dos descritores de grupo
void load_group_desc(ext2_info* fs_info) {
    // cria o aux de numeros de bloco e blocos por grupo, -1 para melhorar na divisao abaixo
    // C sempre arredonda pra baixo, podendo causar inconsistencia
    unsigned int aux = fs_info->sb.s_blocks_count + fs_info->sb.s_blocks_per_group - 1;
    // aux foi feito so pra ficar mais facil de ler
    // calcula o numero de descritores de grupos
    fs_info->num_block_groups = aux / fs_info->sb.s_blocks_per_group;

    // alocar a memoria para os descritores de grupo
    fs_info->group_desc_array = (group_desc*)malloc(fs_info->num_block_groups * sizeof(group_desc));
    // indicar que deu ruim
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
    // le diretamenente no array de group desc as informacoes de n grupos

    // definir diretorio raiz
    fs_info->current_dir_inode = 2; // inode 2 é sempre o diretório raiz
    strcpy(fs_info->current_path, "/");
}

inode_struct read_inode_by_number(ext2_info* fs_info, unsigned int inode_number) {
    // divide o numero do inode pelo numero de inodes por grupo para saber me qual grupo esta
    int group = (inode_number - 1) / fs_info->sb.s_inodes_per_group;
    // pega o descritor do grupo que o inode esta
    group_desc group_desc = fs_info->group_desc_array[group];
    // faz o calculo com % para saber qual a posição do inode dentro do grupo
    int inode_index_on_group = (inode_number - 1) % fs_info->sb.s_inodes_per_group;
    // pega o inicio da tabela de inodes do descritor de grupo
    int initial_position_inode_table = group_desc.bg_inode_table * fs_info->block_size;
    // pega a posiçao atual do inode informado
    int final_position_of_inode = initial_position_inode_table + (inode_index_on_group * fs_info->sb.s_inode_size);

    // monta a struct do inode
    inode_struct inode;
    // anda ate a posiçao atual no arquivo
    lseek(fs_info->fd, final_position_of_inode, SEEK_SET);
    // le o inode atual para struct
    read(fs_info->fd, &inode, sizeof(inode_struct));
    // retorna o inode
    return inode;
}

// funcao auxiliar para evitar repetiçao de codigo
void write_inode_by_number(ext2_info* fs_info, unsigned int inode_number, inode_struct* new_inode) {
    // divide o numero do inode pelo numero de inodes por grupo para saber me qual grupo esta
    int group = (inode_number - 1) / fs_info->sb.s_inodes_per_group;
    // pega o descritor do grupo que o inode esta
    group_desc group_desc = fs_info->group_desc_array[group];
    // faz o calculo com % para saber qual a posição do inode dentro do grupo
    int inode_index_on_group = (inode_number - 1) % fs_info->sb.s_inodes_per_group;
    // pega o inicio da tabela de inodes do descritor de grupo
    int initial_position_inode_table = group_desc.bg_inode_table * fs_info->block_size;
    // pega a posiçao atual do inode informado
    int final_position_of_inode = initial_position_inode_table + (inode_index_on_group * fs_info->sb.s_inode_size);

    // anda ate a posiçao atual no arquivo
    lseek(fs_info->fd, final_position_of_inode, SEEK_SET);
    // salva o novo inode na memoria
    write(fs_info->fd, new_inode, sizeof(inode_struct));
}

// adiciona uma nova dir entry no datablock do inode "pai"
// tem a variavel commit_changes para ter um dry-run, usado para verificar se tem tamanho para adicionar
int add_dir_entry(ext2_info* fs_info, unsigned int parent_inode_num, unsigned int new_inode_num, char* filename,
                  int file_type, bool commit_changes) { // vlw magalu pelo commit_changes
    // le o inode "pai" pelo numero passado
    inode_struct parent_inode = read_inode_by_number(fs_info, parent_inode_num);

    // le o data block do inode
    char block_buffer[fs_info->block_size];
    read_data_block(fs_info, parent_inode.i_block[0], block_buffer, sizeof(block_buffer));

    // calcula o tamanho da entrada necessario
    // 8 sendo o fixo da struct + o tamanho do nome e o arredondamento para multiplo de 4
    int required_size_for_new_entry = (8 + strlen(filename) + 3) & ~3;

    // coloca o ponteiro para a posiçao inicial do block_buffer
    char* pointer = block_buffer;
    int bytes_read = 0;

    while (bytes_read < fs_info->block_size) {
        dir_entry* current_entry = (dir_entry*)pointer;

        // calcula o tamanho que a entrada atual precisa de verdade
        // 8 fixo + tamanho e arredonda pra 4
        int real_size_of_current_entry = (8 + current_entry->name_len + 3) & ~3;

        // encontra o espaço vazio criado pelo rm, ou outros motivos
        int empty_space = current_entry->rec_len - real_size_of_current_entry;

        // se o espaço vazio for o suficiente para encaixar o novo
        if (empty_space >= required_size_for_new_entry) {
            // insere o novo aq

            if (!commit_changes) return 1;
            // guarda o reclen da entrada que tinha o buraco
            int original_rec_len = current_entry->rec_len;

            // 1. encolhe a entrada para o tamanho real necessário
            current_entry->rec_len = real_size_of_current_entry;

            // 2. avança o ponteiro para o começo do espaço livre novo
            char* new_entry_pointer = pointer + current_entry->rec_len;

            // Como esta aproveitando buracos, é necessario limpar o "buraco" por completo
            // isso pq, se a entrada nova for menor que o tamanho total do buraco, ele pode corromper algo
            // e outro pode ser criado no mesmo buraco
            memset(new_entry_pointer, 0, original_rec_len - real_size_of_current_entry);

            // criar a nova entrada
            dir_entry* new_entry = (dir_entry*)new_entry_pointer;
            new_entry->inode = new_inode_num;
            new_entry->name_len = strlen(filename);
            memcpy(new_entry->name, filename, new_entry->name_len);
            new_entry->file_type = file_type;

            // rec len da nova entrada vai ser o tamanho do buraco achado
            new_entry->rec_len = original_rec_len - current_entry->rec_len;

            point_and_write(fs_info->fd, parent_inode.i_block[0] * fs_info->block_size, SEEK_SET,
                            block_buffer, fs_info->block_size);

            // indica que é possivel, e se for commit=true ja foi cadastrada
            return 1;
        }


        // caso o rec_len desta entrada nos leva exatamente até o final do bloco,
        // então ele é a última entrada.
        bytes_read += current_entry->rec_len;
        // mexe o ponteiro para frente
        pointer += current_entry->rec_len;
    }

    // nao tem espaço no bloco.
    printf("Erro: Não há espaço no diretório para criar o novo arquivo/diretorio.\n");
    // todo padronizar isso mais tarde
    return 0;
}


// remove uma dir entry no datablock do inode "pai"
int remove_dir_entry(ext2_info* fs_info, unsigned int parent_inode_num, char* filename_to_remove) {
    // carrega o inode pelo numero informado
    inode_struct parent_inode = read_inode_by_number(fs_info, parent_inode_num);

    // carrega o data block para o buffer
    char block_buffer[fs_info->block_size];
    read_data_block(fs_info, parent_inode.i_block[0], block_buffer, sizeof(block_buffer));

    // passa a posiçao que veio do buffer para o pointer
    char* pointer = block_buffer;
    int bytes_read = 0;
    // cria as variaveis de entry atual e anterior
    dir_entry* current_entry = NULL;
    dir_entry* previous_entry = NULL;

    // loop ate que os bytes sejam maiores que o tamanho do bloco
    while (bytes_read < fs_info->block_size) {
        // monta o dir_entry usando o ponteiro
        current_entry = (dir_entry*)pointer;

        // se tiver reclen == 0 é pq tem algo errado
        if (current_entry->rec_len == 0) break;

        // se tiver inode e o nome do arquivo for igual a da entry e o tamanho também
        if (current_entry->inode != 0 &&
            strncmp(filename_to_remove, current_entry->name, current_entry->name_len) == 0 &&
            strlen(filename_to_remove) == current_entry->name_len) {
            // esticar o rec_len da entrada anterior
            current_entry->inode = 0;
            previous_entry->rec_len += current_entry->rec_len;

            point_and_write(fs_info->fd, parent_inode.i_block[0] * fs_info->block_size, SEEK_SET,
                            block_buffer, fs_info->block_size);
            return 1; // sucessp
        }

        // caso o rec_len desta entrada nos leva exatamente até o final do bloco,
        // então ele é a última entrada

        // seta o previous para ser o antigo current
        previous_entry = current_entry;
        bytes_read += current_entry->rec_len;
        pointer += current_entry->rec_len;
    }

    // nao tem espaço no bloco.
    printf("Erro: arquivo não encontrado neste diretorio.\n");
    // todo padronizar isso mais tarde
    return 0;
}

// acha o numero do inode pelo caminho informado
unsigned int find_inode_number_by_path(ext2_info* fs_info, char* path) {
    // variavel para controlar o inode inicial
    unsigned int start_inode;

    // se começar com / entao é o 2(inode raiz)
    if (path[0] == '/') { // Caminho absoluto
        start_inode = 2;
    } else { // se não começa com o inode cadastrado como o atual
        start_inode = fs_info->current_dir_inode;
    }
    // começa a splitar o path
    char* splited_path = strtok(path, "/");

    // enquanto não for nulo(acabar as /)
    while (splited_path != NULL) {
        // le o inode pelo numero que esta no start
        inode_struct inode = read_inode_by_number(fs_info, start_inode);

        // verifica se é ou não diretorio (podem tentar trollar, colocar um arquivo no path)
        if (!is_dir(inode.i_mode)) { // não é um diretorio.
            printf("Erro: '%s' não é um diretório no caminho.\n", "componente_anterior"); // Melhorar isso depois
            return 0;
        }

        // ler o data block do diretorio atual
        char tmp[1024];
        read_data_block(fs_info, inode.i_block[0], tmp, sizeof(tmp));

        // faz o loop pelo diretorio
        char* actual_pointer = tmp;
        int bytes_read = 0;
        unsigned int next_inode = 0;

        // ler ate o bytes passar do tamanho do bloco
        while (bytes_read < fs_info->block_size) {
            // pega o dir_entry atual
            dir_entry* entry = (dir_entry*)actual_pointer;

            // se o tamanho do diretorio é 0 tem algo errado
            if (entry->rec_len == 0) { break; }

            // 0 significa excluido ou vazio
            if (entry->inode != 0) {
                // comparar o nome da entrada com o caminho atual e o tamanho
                if (strncmp(splited_path, entry->name, entry->name_len) == 0 &&
                    strlen(splited_path) == entry->name_len) {
                    next_inode = entry->inode;
                    break;
                }
            }
            actual_pointer += entry->rec_len;
            bytes_read += entry->rec_len;
        }

        // se não achou o inode no loop
        if (next_inode == 0) {
            printf("Erro: '%s' não encontrado.\n", splited_path);
            return 0;
        }

        // vai trocando o inode a ser buscado
        start_inode = next_inode;

        // continua o split do path
        splited_path = strtok(NULL, "/");
    }
    return start_inode;
}

unsigned int find_parent_inode_and_final_name(ext2_info* fs_info, const char* full_path, char* final_name_out) {
    char path_copy[1024];
    strcpy(path_copy, full_path);

    // strrchr retorna a  ultima /
    char* ultimo_slash = strrchr(path_copy, '/');

    char parent_path[1024];

    if (ultimo_slash == NULL) { // teste.txt
        // se não tiver, é relativo e esta no diretorio
        strcpy(final_name_out, path_copy);
        return fs_info->current_dir_inode;
    } else if (ultimo_slash == path_copy) { // /teste.txt
        // a unica barra é a primeira, o pai é a raiz
        // inode 2
        strcpy(final_name_out, ultimo_slash + 1); // copia o que vem depois da '/'
        strcpy(parent_path, "/");
    } else { // livros/teste.txt
        strcpy(final_name_out, ultimo_slash + 1);
        *ultimo_slash = '\0'; // corta a string na barra, path_copy agora é "livros"
        strcpy(parent_path, path_copy);
    }

    // Agora que temos o caminho do pai, usamos a função que JÁ EXISTE para achar o inode dele!
    return find_inode_number_by_path(fs_info, parent_path);
}


void read_data_block(ext2_info* fs_info, int block_number, char* buffer, int buffer_size) {
    // endereço do conteudo
    int content_location = block_number * fs_info->block_size;
    lseek(fs_info->fd, content_location, SEEK_SET);
    read(fs_info->fd, buffer, buffer_size);
}

unsigned int allocate_item(ext2_info* fs_info, char type) {
    // loop para cada grupo no sistema de arquivos
    for (int i = 0; i < fs_info->num_block_groups; ++i) {
        // bitmap esta dentro do descritor de grupo
        group_desc gd = fs_info->group_desc_array[i];

        // configuração por tipo
        unsigned int free_items_in_group;
        unsigned int bitmap_block_num;
        unsigned int items_per_group;

        if (type == 'i') {
            // configura para inode
            free_items_in_group = gd.bg_free_inodes_count;
            bitmap_block_num = gd.bg_inode_bitmap;
            items_per_group = fs_info->sb.s_inodes_per_group;
        } else { // type == 'b'
            // configura para bloco
            free_items_in_group = gd.bg_free_blocks_count;
            bitmap_block_num = gd.bg_block_bitmap;
            items_per_group = fs_info->sb.s_blocks_per_group;
        }

        // Se o grupo nao tiver blocos ou inodes, continua
        if (free_items_in_group == 0) continue;

        // unsigned int block_bitmap_block_num = gd.bg_block_bitmap;
        char bitmap_buffer[fs_info->block_size];
        read_data_block(fs_info, bitmap_block_num, bitmap_buffer, sizeof(bitmap_buffer));

        // loop para cada BYTE do bitmap
        for (int byte_index = 0; byte_index < fs_info->block_size; ++byte_index) {
            // loop para cada BIT dentro do byte atual (8 bits por byte)
            for (int bit_idx = 0; bit_idx < 8; bit_idx++) {
                // verifica se o bit foi setado ou nao
                if (!(bitmap_buffer[byte_index] & (1 << (bit_idx)))) {
                    // bit livre
                    // alocar o bloco
                    // calcular o numero do bloco baseado na posição do bitmap
                    // soma os blocos dos grupos anteriores + os bits dos bytes anteriores + o bit atual + 1 para achar o ID.
                    unsigned int item_number = (i * items_per_group) + (byte_index * 8) + (bit_idx + 1);

                    // me lembro de fazer isso em microcontroladores
                    bitmap_buffer[byte_index] |= (1 << (bit_idx)); // setar bit como 1(usado)

                    // salvar o bitmap
                    point_and_write(fs_info->fd, bitmap_block_num * fs_info->block_size, SEEK_SET,
                                    bitmap_buffer, fs_info->block_size);

                    // alterar os contadores de blocos ou inodes free
                    if (type == 'i') {
                        fs_info->sb.s_free_inodes_count--;
                        fs_info->group_desc_array[i].bg_free_inodes_count--;
                    } else {
                        fs_info->sb.s_free_blocks_count--;
                        fs_info->group_desc_array[i].bg_free_blocks_count--;
                    }

                    // salvar no disco as informações
                    point_and_write(fs_info->fd, 1024, SEEK_SET, &fs_info->sb, sizeof(super_block));

                    // salvar o descritor de grupo
                    // calcular a posiçao exata em bytes do descritor de grupo
                    // block size * 2 é pq a tabela esta no segundo bloco do sistema
                    // pula i group descs para asber qual modificar
                    unsigned int group_desc_position = fs_info->block_size * 2 + (i * sizeof(group_desc));
                    point_and_write(fs_info->fd, group_desc_position, SEEK_SET,
                                    &(fs_info->group_desc_array[i]), sizeof(group_desc));

                    return item_number;
                }
            }
        }
    }

    if (type == 'i') printf("Erro: Sistema sem inodes livres\n");
    else printf("Erro: Sistema sem blocos livres\n");
    return 0; // retorna 0 se não encontrou nada em nenhum grupo
}

void deallocate_item(ext2_info* fs_info, unsigned int item_number, char type) {
    unsigned int items_per_group;
    if (type == 'i') {
        items_per_group = fs_info->sb.s_inodes_per_group;
    } else {
        items_per_group = fs_info->sb.s_blocks_per_group;
    }

    unsigned int group_index = (item_number - 1) / items_per_group;
    unsigned int local_index = (item_number - 1) % items_per_group;
    unsigned int byte_index = local_index / 8;
    unsigned int bit_index = local_index % 8;

    unsigned int bitmap_block_num;
    if (type == 'i') {
        bitmap_block_num = fs_info->group_desc_array[group_index].bg_inode_bitmap;
    } else {
        bitmap_block_num = fs_info->group_desc_array[group_index].bg_block_bitmap;
    }

    char bitmap_buffer[fs_info->block_size];
    read_data_block(fs_info, bitmap_block_num, bitmap_buffer, sizeof(bitmap_buffer));

    // setar o bit como 0 (livre)
    // operaçao inversa do que foi feito la em cima |=
    bitmap_buffer[byte_index] &= ~(1 << bit_index);

    // salvar o bitmap modificado no disco

    point_and_write(fs_info->fd, bitmap_block_num * fs_info->block_size, SEEK_SET, // lseek
                    bitmap_buffer, fs_info->block_size); // write

    // atualzar e salvar na memoria os contadores
    if (type == 'i') {
        fs_info->sb.s_free_inodes_count++;
        fs_info->group_desc_array[group_index].bg_free_inodes_count++;
    } else {
        fs_info->sb.s_free_blocks_count++;
        fs_info->group_desc_array[group_index].bg_free_blocks_count++;
    }

    point_and_write(fs_info->fd, 1024, SEEK_SET, &fs_info->sb, sizeof(super_block));

    // salvar o descritor de grupo
    // calcular a posiçao exata em bytes do descritor de grupo
    // block size * 2 é pq a tabela esta no segundo bloco do sistema
    // pula o 'group_index' de group descs para saber qual modificar
    unsigned int group_desc_position = fs_info->block_size * 2 + (group_index * sizeof(group_desc));
    point_and_write(fs_info->fd, group_desc_position, SEEK_SET,
                    &(fs_info->group_desc_array[group_index]), sizeof(group_desc));
}


bool verify_file_exists(ext2_info* fs_info, unsigned int i_block, char* file_name) {
    // percorrer o data block verificando se o arquivo existe
    char tmp[1024];
    read_data_block(fs_info, i_block, tmp, sizeof(tmp));

    char* actual_pointer = tmp;
    int bytes_read = 0;

    while (bytes_read < BASE_BLOCK) {
        dir_entry* entry = (dir_entry*)actual_pointer;

        if (entry->rec_len == 0) break;

        // 0 significa excluido ou vazio
        if (entry->inode != 0) {
            if (strncmp(file_name, entry->name, entry->name_len) == 0 && strlen(file_name) == entry->name_len) {
                return true;
            }
        }
        actual_pointer += entry->rec_len;
        bytes_read += entry->rec_len;
    }
    return false;
}
