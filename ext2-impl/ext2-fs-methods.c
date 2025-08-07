//
// Created by moraski on 07/06/25.
//
#include "ext2-fs-methods.h"

#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "../entities_struct/entities.h"
#include "../entities_struct/metadata-dict.h"
#include "../utils/utils.h"

ext2_info mount_ext2_info() {
    ext2_info fs_info;
    int fd;
    if ((fd = open(IMG_PATH, O_RDWR)) < 0) {
        perror(IMG_PATH);
        exit(EXIT_FAILURE);
    }

    fs_info.fd = fd;
    load_super_block(&fs_info);
    load_group_desc(&fs_info);
    return fs_info;
}


// carrega as informações do superbloco
void load_super_block(ext2_info *fs_info) {
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
void load_group_desc(ext2_info *fs_info) {
    // cria o aux de numeros de bloco e blocos por grupo, -1 para melhorar na divisao abaixo
    // C sempre arredonda pra baixo, podendo causar inconsistencia
    unsigned int aux = fs_info->sb.s_blocks_count + fs_info->sb.s_blocks_per_group - 1;
    // aux foi feito so pra ficar mais facil de ler
    // calcula o numero de descritores de grupos
    fs_info->num_block_groups = aux / fs_info->sb.s_blocks_per_group;

    // alocar a memoria para os descritores de grupo
    fs_info->group_desc_array = (group_desc *) malloc(fs_info->num_block_groups * sizeof(group_desc));
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

inode_struct read_inode_by_number(ext2_info *fs_info, unsigned int inode_number) {
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
void write_inode_by_number(ext2_info *fs_info, unsigned int inode_number, inode_struct *new_inode) {
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
int add_dir_entry(ext2_info *fs_info, unsigned int parent_inode_num, unsigned int new_inode_num, char *filename,
                  int file_type, bool commit_changes) {
    // vlw magalu pelo commit_changes
    // le o inode "pai" pelo numero passado
    inode_struct parent_inode = read_inode_by_number(fs_info, parent_inode_num);

    // le o data block do inode
    char block_buffer[fs_info->block_size];
    read_data_block(fs_info, parent_inode.i_block[0], block_buffer, sizeof(block_buffer));

    // calcula o tamanho da entrada necessario
    // 8 sendo o fixo da struct + o tamanho do nome e o arredondamento para multiplo de 4
    int required_size_for_new_entry = (8 + strlen(filename) + 3) & ~3;

    // coloca o ponteiro para a posiçao inicial do block_buffer
    char *pointer = block_buffer;
    int bytes_read = 0;

    while (bytes_read < fs_info->block_size) {
        dir_entry *current_entry = (dir_entry *) pointer;

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
            char *new_entry_pointer = pointer + current_entry->rec_len;

            // Como esta aproveitando buracos, é necessario limpar o "buraco" por completo
            // isso pq, se a entrada nova for menor que o tamanho total do buraco, ele pode corromper algo
            // e outro pode ser criado no mesmo buraco
            memset(new_entry_pointer, 0, original_rec_len - real_size_of_current_entry);

            // criar a nova entrada
            dir_entry *new_entry = (dir_entry *) new_entry_pointer;
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
int remove_dir_entry(ext2_info *fs_info, unsigned int parent_inode_num, char *filename_to_remove) {
    // carrega o inode pelo numero informado
    inode_struct parent_inode = read_inode_by_number(fs_info, parent_inode_num);

    // carrega o data block para o buffer
    char block_buffer[fs_info->block_size];
    read_data_block(fs_info, parent_inode.i_block[0], block_buffer, sizeof(block_buffer));

    // passa a posiçao que veio do buffer para o pointer
    char *pointer = block_buffer;
    int bytes_read = 0;
    // cria as variaveis de entry atual e anterior
    dir_entry *current_entry = NULL;
    dir_entry *previous_entry = NULL;

    // loop ate que os bytes sejam maiores que o tamanho do bloco
    while (bytes_read < fs_info->block_size) {
        // monta o dir_entry usando o ponteiro
        current_entry = (dir_entry *) pointer;

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
unsigned int find_inode_number_by_path(ext2_info *fs_info, char *path) {
    // variavel para controlar o inode inicial
    unsigned int start_inode;

    // se começar com / entao é o 2(inode raiz)
    if (path[0] == '/') {
        // Caminho absoluto
        start_inode = 2;
    } else {
        // se não começa com o inode cadastrado como o atual
        start_inode = fs_info->current_dir_inode;
    }
    // começa a splitar o path
    char *splited_path = strtok(path, "/");

    // enquanto não for nulo(acabar as /)
    while (splited_path != NULL) {
        // le o inode pelo numero que esta no start
        inode_struct inode = read_inode_by_number(fs_info, start_inode);

        // verifica se é ou não diretorio (podem tentar trollar, colocar um arquivo no path)
        if (!is_dir(inode.i_mode)) {
            // não é um diretorio.
            printf("Erro: '%s' não é um diretório no caminho.\n", "componente_anterior"); // Melhorar isso depois
            return 0;
        }

        // ler o data block do diretorio atual
        char tmp[1024];
        read_data_block(fs_info, inode.i_block[0], tmp, sizeof(tmp));

        // faz o loop pelo diretorio
        char *actual_pointer = tmp;
        int bytes_read = 0;
        unsigned int next_inode = 0;

        // ler ate o bytes passar do tamanho do bloco
        while (bytes_read < fs_info->block_size) {
            // pega o dir_entry atual
            dir_entry *entry = (dir_entry *) actual_pointer;

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
        if (next_inode == 0) return 0;

        // vai trocando o inode a ser buscado
        start_inode = next_inode;

        // continua o split do path
        splited_path = strtok(NULL, "/");
    }
    return start_inode;
}

// Separa o path para pegar apenas o final do caminho, ultimo nome passado e dps retorna o numero do "pai"
// foi usado para funções que precisavam chegar ate um caminho mas o final era o nome do arquivo que ainda n existe
unsigned int find_parent_inode_and_filename(ext2_info *fs_info, const char *full_path, char *filename_out) {
    char path_copy[1024];
    strcpy(path_copy, full_path);

    // strrchr retorna a  ultima /
    char *ultimo_slash = strrchr(path_copy, '/');

    char parent_path[1024];

    if (ultimo_slash == NULL) {
        // teste.txt
        // se não tiver, é relativo e esta no diretorio
        strcpy(filename_out, path_copy);
        return fs_info->current_dir_inode;
    } else if (ultimo_slash == path_copy) {
        // /teste.txt
        // a unica barra é a primeira, o pai é a raiz
        // inode 2
        strcpy(filename_out, ultimo_slash + 1); // copia o que vem depois da '/'
        strcpy(parent_path, "/");
    } else {
        // livros/teste.txt
        strcpy(filename_out, ultimo_slash + 1);
        *ultimo_slash = '\0'; // corta a string na barra, path_copy agora é "livros"
        strcpy(parent_path, path_copy);
    }

    // Agora que temos o caminho do pai, usamos a função que JÁ EXISTE para achar o inode dele!
    return find_inode_number_by_path(fs_info, parent_path);
}


void read_data_block(ext2_info *fs_info, int block_number, char *buffer, int buffer_size) {
    // endereço do conteudo
    int content_location = block_number * fs_info->block_size;
    // aponta para a posição do conteudo e le o tamanho do buffer
    lseek(fs_info->fd, content_location, SEEK_SET);
    read(fs_info->fd, buffer, buffer_size);
}

// função generica para alocação de item, diferenciado pelo tipo, podendo ser i(i-node) e b(bloco)
// variaveis de controle dos contadores sao armazenadas baseado no tipo passado
// aloca um novo inode e retorna erro(0) ou o número do novo inode
unsigned int allocate_item(ext2_info *fs_info, char type) {
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
        } else {
            // type == 'b'
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

// função genericap ara desalocação do item, podendo ser inode(i) ou bloco(b)
// as variaveis de contagem sao atualizadas baseado no tipo passado
void deallocate_item(ext2_info *fs_info, unsigned int item_number, char type) {
    // variavel de controle de itens por grupo
    unsigned int items_per_group;
    // pega o tipo e preenche a variavel com o valor especifico
    if (type == 'i') {
        items_per_group = fs_info->sb.s_inodes_per_group;
    } else {
        items_per_group = fs_info->sb.s_blocks_per_group;
    }

    // -1 é necessário para garantir que não cause problemas com numeros proximos ao limite
    // encontrar o grupo que o item esta, em qual grupo o inode esta
    unsigned int group_index = (item_number - 1) / items_per_group;
    // descobre a posição do item dentro do grupo
    // é feito a operação mod para encontrar, como descobrir qual indice do bloco 21 em 100 blocos
    // 21 - 1 % 100 = 20 -> indice 20
    unsigned int local_index = (item_number - 1) % items_per_group;
    // encontrar o byte e bit no bitmap
    // como sabemos a posiçao, precisamos achar o byte, então dividimos a posição por 8 para achar o byte do mapa onde esta
    unsigned int byte_index = local_index / 8;
    // encontra em qual bit esta a informação dentro do byte
    unsigned int bit_index = local_index % 8;

    // variavel para armazenar o bitmap
    unsigned int bitmap_block_num;
    // pega o bitmap baseado no index de grupo e no tipo
    if (type == 'i') {
        bitmap_block_num = fs_info->group_desc_array[group_index].bg_inode_bitmap;
    } else {
        bitmap_block_num = fs_info->group_desc_array[group_index].bg_block_bitmap;
    }

    // le o bitmap para o buffer
    char bitmap_buffer[fs_info->block_size];
    read_data_block(fs_info, bitmap_block_num, bitmap_buffer, sizeof(bitmap_buffer));

    // setar o bit como 0 (livre)
    // operaçao inversa do que foi feito la em cima |=
    // na posiçao do byte especifico no bitmap, troca o bit de 1 para 0(indicar como livre)
    bitmap_buffer[byte_index] &= ~(1 << bit_index);

    // salvar o bitmap modificado no disco
    point_and_write(fs_info->fd, bitmap_block_num * fs_info->block_size, SEEK_SET, // lseek
                    bitmap_buffer, fs_info->block_size); // write

    // atualizar e salvar na memoria os contadores baseado no tipo
    if (type == 'i') {
        fs_info->sb.s_free_inodes_count++;
        fs_info->group_desc_array[group_index].bg_free_inodes_count++;
    } else {
        fs_info->sb.s_free_blocks_count++;
        fs_info->group_desc_array[group_index].bg_free_blocks_count++;
    }

    // salva o superbloco das informações novas(free blocks, inodes
    point_and_write(fs_info->fd, 1024, SEEK_SET, &fs_info->sb, sizeof(super_block));

    // salvar o descritor de grupo
    // calcular a posiçao exata em bytes do descritor de grupo
    // block size * 2 é pq a tabela esta no segundo bloco do sistema
    // pula o 'group_index' de group descs para saber qual modificar
    unsigned int group_desc_position = fs_info->block_size * 2 + (group_index * sizeof(group_desc));
    point_and_write(fs_info->fd, group_desc_position, SEEK_SET,
                    &(fs_info->group_desc_array[group_index]), sizeof(group_desc));
}


// função auxiliar para verificar se o arquivo existe no sistema de arquivos
bool verify_file_exists(ext2_info *fs_info, unsigned int i_block, char *filename) {
    // le o datablock baseado no i_block passado
    char tmp[1024];
    read_data_block(fs_info, i_block, tmp, sizeof(tmp));


    // percorrer o data block verificando se o arquivo existe
    char *actual_pointer = tmp;
    int bytes_read = 0;

    while (bytes_read < BASE_BLOCK) {
        dir_entry *entry = (dir_entry *) actual_pointer;

        if (entry->rec_len == 0) break;

        // 0 significa excluido ou vazio
        if (entry->inode != 0) {
            if (strncmp(filename, entry->name, entry->name_len) == 0 && strlen(filename) == entry->name_len) {
                return true;
            }
        }
        actual_pointer += entry->rec_len;
        bytes_read += entry->rec_len;
    }
    return false;
}


unsigned int get_block_number_by_index(ext2_info *fs_info, inode_struct *target_inode, unsigned int index) {
    if (index < 12) {
        return target_inode->i_block[index];
    }
    if (index < 12 + 256) {
        if (target_inode->i_block[12] == 0) return 0;
        unsigned int indirect_pointers[256];
        read_data_block(fs_info, target_inode->i_block[12], (char *) indirect_pointers, sizeof(indirect_pointers));
        int indirect_block_index = index - 12;
        return indirect_pointers[indirect_block_index];
    }
    if (target_inode->i_block[13] == 0) return 0;
    unsigned int l1_indirect_pointers[256];
    read_data_block(fs_info, target_inode->i_block[13], (char *) l1_indirect_pointers, sizeof(l1_indirect_pointers));
    int adjust_index = index - (12 + 256);
    unsigned int l2_block_index = l1_indirect_pointers[adjust_index / 256];
    if (l2_block_index == 0) return 0;
    unsigned int l2_indirect_pointers[256];
    read_data_block(fs_info, l2_block_index, (char *) l2_indirect_pointers, sizeof(l2_indirect_pointers));
    return l2_indirect_pointers[adjust_index % 256];
}

unsigned int save_buffer_and_register_block(ext2_info *fs_info, inode_struct *target_inode, char *block_buffer,
                                            unsigned int block_index) {
    unsigned int existent_block_num = get_block_number_by_index(fs_info, target_inode, block_index);

    // se não existe aloca um novo
    if (existent_block_num == 0) {
        existent_block_num = allocate_item(fs_info, 'b');
        if (existent_block_num == 0) {
            printf("Erro fatal: Falha ao alocar bloco de dados. Disco pode estar cheio.\n");
            return 0;
        }

        // blocos Diretos
        if (block_index < 12) {
            target_inode->i_block[block_index] = existent_block_num;
        }
        // Bloco Indireto Simples
        else if (block_index < 12 + 256) {
            unsigned int indirect_block_num = target_inode->i_block[12];
            unsigned int indirect_pointers[256];

            // se o bloco de ponteiros indiretos ainda não existe, cria ele.
            if (indirect_block_num == 0) {
                indirect_block_num = allocate_item(fs_info, 'b');
                if (indirect_block_num == 0) return 0; // falhou em alocar o bloco de ponteiros
                target_inode->i_block[12] = indirect_block_num;
                memset(indirect_pointers, 0, sizeof(indirect_pointers)); // limpa o buffer de ponteiros
            } else {
                // se ja existe, le ele do disco para a memória.
                read_data_block(fs_info, indirect_block_num, (char *) indirect_pointers, fs_info->block_size);
            }

            int index_no_bloco = block_index - 12;
            indirect_pointers[index_no_bloco] = existent_block_num;

            // salva a lista de ponteiros atualizada de volta no disco.
            point_and_write(fs_info->fd, indirect_block_num * fs_info->block_size, SEEK_SET, (char *) indirect_pointers,
                            fs_info->block_size);
        }
        // Bloco Indireto Duplo
        else if (block_index < 12 + 256 + (256 * 256)) {
            unsigned int l1_block_num = target_inode->i_block[13];
            unsigned int l1_pointers[256];

            if (l1_block_num == 0) {
                l1_block_num = allocate_item(fs_info, 'b');
                if (l1_block_num == 0) return 0;
                target_inode->i_block[13] = l1_block_num;
                memset(l1_pointers, 0, sizeof(l1_pointers));
            } else {
                read_data_block(fs_info, l1_block_num, (char *) l1_pointers, fs_info->block_size);
            }

            int adjuted_index = block_index - (12 + 256);
            int l1_index = adjuted_index / 256;
            int l2_index = adjuted_index % 256;

            unsigned int l2_block_num = l1_pointers[l1_index];
            unsigned int l2_pointers[256];

            if (l2_block_num == 0) {
                l2_block_num = allocate_item(fs_info, 'b');
                if (l2_block_num == 0) return 0;
                l1_pointers[l1_index] = l2_block_num;
                point_and_write(fs_info->fd, l1_block_num * fs_info->block_size, SEEK_SET, (char *) l1_pointers,
                                fs_info->block_size);
                memset(l2_pointers, 0, sizeof(l2_pointers));
            } else {
                read_data_block(fs_info, l2_block_num, (char *) l2_pointers, fs_info->block_size);
            }

            l2_pointers[l2_index] = existent_block_num;
            point_and_write(fs_info->fd, l2_block_num * fs_info->block_size, SEEK_SET, (char *) l2_pointers,
                            fs_info->block_size);
        } else {
            printf("Erro: Arquivo muito grande, escrita em blocos indiretos triplos não suportada.\n");
            return 0;
        }
    }
    point_and_write(fs_info->fd, existent_block_num * fs_info->block_size, SEEK_SET, block_buffer, fs_info->block_size);

    return 1; // Sucesso
}

void write_data_block_out(ext2_info *fs_info, unsigned int block_number, char block_buffer[], long total_length,
                          long *bytes_read, FILE *target_file) {
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
void print_data_block(ext2_info *fs_info, unsigned int block_number, char *block_buffer, long *total_length,
                      long *bytes_read) {
    read_data_block(fs_info, block_number, block_buffer, fs_info->block_size);
    // evitar printar lixo de memoria
    // calcular quantos bytes ainda faltam para ser lido o arquivo
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

void parse_and_print_records(char *block_buffer, int bytes_in_buffer, long *total_bytes_to_read, int record_size,
                             int *limit_counter) {
    for (int offset = 0; offset < bytes_in_buffer; offset += record_size) {
        if (*total_bytes_to_read <= 0 || *limit_counter <= 0) break;

        UserRecord *current_record = (UserRecord *) (block_buffer + offset);

        if (current_record->id != 0) {
            printf("id:%u;is_active:%d;username:%s;email:%s\n",
                   current_record->id,
                   current_record->is_active,
                   current_record->username,
                   current_record->email);

            if (*limit_counter > 0) (*limit_counter)--;
        }

        (*total_bytes_to_read) -= record_size;
    }
}

void parse_and_print_records_where(char *block_buffer, int bytes_in_buffer, long *total_bytes_to_read, int record_size,
                                   int *limit_counter, const FieldMetadata *field_metadata, char *value, char* operator) {
    for (int offset = 0; offset < bytes_in_buffer; offset += record_size) {
        if (*total_bytes_to_read <= 0 || *limit_counter <= 0) break;

        UserRecord *current_record = (UserRecord *) (block_buffer + offset);

        char *field_ptr = (char *) current_record + field_metadata->offset;

        bool match = false;

        switch (field_metadata->type) {
            case FIELD_TYPE_UINT32:
                if (*(uint32_t *) field_ptr == atoi(value)) {
                    match = true;
                }
                break;
            case FIELD_TYPE_STRING:
                if (strcmp(operator, "=") == 0) {
                    if (strcmp(field_ptr, value) == 0) {
                        match = true;
                    }
                } else if (strcmp(operator, "%") == 0) {
                    if (strstr(field_ptr, value) != NULL) {
                        match = true;
                    }
                }
                break;
            case FIELD_TYPE_CHAR:
                if (*field_ptr == value[0]) {
                    match = true;
                }
                break;
        }
        if (match) {
            printf("id:%u;is_active:%d;username:%s;email:%s\n",
                              current_record->id,
                              current_record->is_active,
                              current_record->username,
                              current_record->email);
            if (*limit_counter > 0) (*limit_counter)--;
        }
        (*total_bytes_to_read) -= record_size;
    }
}

void append_stream_to_file(ext2_info *fs_info, inode_struct *target_inode, unsigned int stdin_buffer_size) {
    char block_buffer[fs_info->block_size];
    unsigned int buffer_cursor = target_inode->i_size % fs_info->block_size;

    if (buffer_cursor > 0) {
        // bloco esta parcialmente preenchido
        int last_block_idx = (target_inode->i_size - 1) / fs_info->block_size;
        int last_block_num = get_block_number_by_index(fs_info, target_inode, last_block_idx);
        read_data_block(fs_info, last_block_num, block_buffer, fs_info->block_size);
    } else {
        // bloco novo
        memset(block_buffer, 0, fs_info->block_size);
    }

    char stdin_buffer[stdin_buffer_size];
    size_t bytes_read_from_stdin;

    while ((bytes_read_from_stdin = fread(stdin_buffer, 1, sizeof(stdin_buffer), stdin)) > 0) {
        for (int i = 0; i < bytes_read_from_stdin; i++) {
            block_buffer[buffer_cursor] = stdin_buffer[i];
            buffer_cursor++;
            target_inode->i_size++;

            if (buffer_cursor == fs_info->block_size) {
                int blocos_alocados_ate_agora = (target_inode->i_size - 1) / fs_info->block_size;
                save_buffer_and_register_block(fs_info, target_inode, block_buffer, blocos_alocados_ate_agora);

                buffer_cursor = 0;
                memset(block_buffer, 0, fs_info->block_size);
            }
        }
    }

    if (buffer_cursor > 0) {
        int used_blocks_now = (target_inode->i_size + fs_info->block_size - 1) / fs_info->block_size;
        if (target_inode->i_size == 0) used_blocks_now = 0;
        else used_blocks_now--;

        save_buffer_and_register_block(fs_info, target_inode, block_buffer, used_blocks_now);
    }

    int num_fs_blocks = (target_inode->i_size + fs_info->block_size - 1) / fs_info->block_size;
    if (target_inode->i_size == 0) num_fs_blocks = 0;

    target_inode->i_blocks = num_fs_blocks * (fs_info->block_size / 512);
    target_inode->i_mtime = time(NULL);
    target_inode->i_atime = target_inode->i_mtime;
}
