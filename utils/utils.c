#include "utils.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "../ext2-impl/ext2-defs.h"

// formata a data pro formato exemplo do documento
void format_date(uint32_t timestamp, char* buffer, int buffer_size) {
    time_t brute_time = timestamp;
    struct tm* time_struct = localtime(&brute_time);
    strftime(buffer, buffer_size, "%d/%m/%Y %H:%M", time_struct);
}

// função para arrumar o path, considerando se o inicio é a raiz (/), se caso tenha .. e etc.
void resolve_path_string(char* resolved_path, const char* current_path, char* path_to_resolve) {
    // como o strtok modifica o valor da variavel, fiz uma copia
    char path_to_resolve_copy[1024];
    strcpy(path_to_resolve_copy, path_to_resolve);

    // se o caminho informado é absoluto, o ponto inicial é o / (raiz)
    // se não é o caminho atual
    if (path_to_resolve[0] == '/') {
        strcpy(resolved_path, "/");
    } else {
        strcpy(resolved_path, current_path);
    }

    // quebra o caminho enquanto navegas no componentes "../.." vira "..", ".."
    char* token = strtok(path_to_resolve_copy, "/");

    // loop para iterar esses componentes
    while (token != NULL) {
        if (strcmp(token, "..") == 0) {
            // se é  "..", sobe um nivel no caminho final
            change_path_level(resolved_path);
        } else if (strcmp(token, ".") == 0) {
            // se é ".", não faz nada
        } else {
            // se for um nome de diretorio, adiciona
            // adiciona uma barra caso nao seja na raiz
            if (strcmp(resolved_path, "/") != 0) {
                strcat(resolved_path, "/");
            }
            strcat(resolved_path, token);
        }

        // pega o proximo
        token = strtok(NULL, "/");
    }
}

// troca o nivel do path, pra tras, por exemplo(..)
void change_path_level(char* current_path) {
    // se ja esta na raiz ok
    if (strcmp(current_path, "/") == 0) {
        return;
    }

    // encontra a posiçao da ultima / no caminho
    char* last_slash = strrchr(current_path, '/');

    // se o ultimo / for o primeiro entao o pai do diretorio é a propria raiz
    // ex /livros
    if (last_slash == current_path) {
        // colocar o terminador nulo logo após a primeira '/', transformando "/livros" em "/".
        current_path[1] = '\0';
    } else {
        // ex /livros/classicos
        // adiciona o \0 no lugar da ultima barra
        *last_slash = '\0';
    }
}

// Retorna um boleano caso seja diretorio ou não
bool is_dir(uint16_t i_mode) {
    unsigned int file_type = i_mode & 0xF000; // Isola os 4 bits do tipo do arquivo PPI
    return file_type == 0x4000;
}

// imprimir indicador de diretorio
// foi deixado evidenciado na frente o S_IFDIR porque foi utilizado a biblioteca sys/stat.h para suporte.
void mount_permissions_string(uint16_t i_mode, char* permission_string) {
    unsigned int file_type = i_mode & 0xF000; // Isola os 4 bits do tipo do arquivo PPI

    // comparar o tipo isolado para os valores necessarios
    if (file_type == EXT2_S_IFDIR) { // S_IFDIR
        permission_string[0] = 'd';
    } else if (file_type == EXT2_S_IFREG) { // S_IFREG
        permission_string[0] = 'f';
    } else if (file_type == EXT2_S_IFLNK) { // S_IFLNK
        permission_string[0] = 'l';
    } else { // ignora outros tipos para este trabalho
        permission_string[0] = '-';
    }
    // formato usado como base é o mesmo do ls -l
    // drwxr-xr--
    // d = diretorio, rwx -> dono, r-x -> grupo, r-- -> outros
    // somando 7 = 4 2 1

    // permissoes do dono (user)
    permission_string[1] = (i_mode & EXT2_S_IRUSR) ? 'r' : '-';
    permission_string[2] = (i_mode & EXT2_S_IWUSR) ? 'w' : '-';
    permission_string[3] = (i_mode & EXT2_S_IXUSR) ? 'x' : '-';

    // permissoes do grupo
    permission_string[4] = (i_mode & EXT2_S_IRGRP) ? 'r' : '-';
    permission_string[5] = (i_mode & EXT2_S_IWGRP) ? 'w' : '-';
    permission_string[6] = (i_mode & EXT2_S_IXGRP) ? 'x' : '-';

    // permissoes de outros
    permission_string[7] = (i_mode & EXT2_S_IROTH) ? 'r' : '-';
    permission_string[8] = (i_mode & EXT2_S_IWOTH) ? 'w' : '-';
    permission_string[9] = (i_mode & EXT2_S_IXOTH) ? 'x' : '-';

    // finaliza a string
    permission_string[10] = '\0';
}

// abstração para fazer a escrita em algum ponto especifico da memoria, passa o numero de bytes a escrever
// e o buffer
void point_and_write(int fd, long offset, int whence, const void* buffer, size_t bytes_to_write) {
    lseek(fd, offset, whence);
    write(fd, buffer, bytes_to_write);
}

// faz o parse do input para considerar se o argumento esta entre aspas
// ex mkdir "moraski lunkes".
int parse_input(char* input, char** args) {
    int argc = 0;
    char* arg_start = NULL; // ponteiro do inicio do argumento
    bool in_quotes = false; // flag para controlar se esta dentro de aspas
    int len = strlen(input); // pega o tamanho da string do input

    // verificar caracter por caracter da string
    for (int i = 0; i < len; i++) {

        char char_atual = input[i]; // pega o caracter atual

        if (char_atual == '\"') {
            input[i] = '\0'; // aspa vira o fim da string
            // se tava nao ta mais, se nao tava agr ta
            in_quotes = !in_quotes; // inverte a condicional

            if (in_quotes) { // se entrou nas aspas
                // passa o argumento da posição de memoria
                arg_start = &input[i + 1]; // indica que o argumento começa no proximo caracter.
            } else { // se saiu das aspas ou não estava nas aspas
                args[argc++] = arg_start; // guarda o argumento
                arg_start = NULL; // Reseta o marcador de início.
            }

        } else if (char_atual == ' ' && !in_quotes) { // se é um espaço e não estamos nas aspas
            input[i] = '\0'; // também o fim do argumento
            // terminou de ler o argumento passado
            if (arg_start != NULL) { // se ja tinha algum argumento pra ler
                // como setamos \0 no input ao fim do argumento
                // outros comandos do C vao procurar pelo endereço na memoria e parar no \0
                args[argc++] = arg_start; // guarda ele
                arg_start = NULL; // reseta
            }
        } else { // caracter normal
            if (arg_start == NULL) { // se não tiver o inicio de um algumento ainda
                arg_start = &input[i]; // indica que o inicio é aq
            }
        }
    }

    // Depois que o loop acaba, pode ter sobrado um último argumento
    if (arg_start != NULL) {
        args[argc++] = arg_start;
    }

    // Finaliza o array de argumentos com NULL, como é o padrão.
    args[argc] = NULL;
    return argc;
}
