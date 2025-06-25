#include "utils.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

void format_date(uint32_t timestamp, char* buffer, int buffer_size) {
    time_t brute_time = timestamp;
    struct tm* time_struct = localtime(&brute_time);
    strftime(buffer, buffer_size, "%d/%m/%Y %H:%M", time_struct);
}

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
    if (file_type == 0x4000) { // S_IFDIR
        permission_string[0] = 'd';
    } else if (file_type == 0x8000) { // S_IFREG
        permission_string[0] = 'f';
    } else if (file_type == 0xA000) { // S_IFLNK
        permission_string[0] = 'l';
    } else {
        permission_string[0] = '-';
    }


    // formato usado como base é o mesmo do ls -l
    // drwxr-xr--
    // d = diretorio, rwx -> dono, r-x -> grupo, r-- -> outros
    // somando 7 = 4 2 1

    // permissoes do dono (user)
    permission_string[1] = (i_mode & 0400) ? 'r' : '-';
    permission_string[2] = (i_mode & 0200) ? 'w' : '-';
    permission_string[3] = (i_mode & 0100) ? 'x' : '-';

    // permissoes do grupo
    permission_string[4] = (i_mode & 0040) ? 'r' : '-';
    permission_string[5] = (i_mode & 0020) ? 'w' : '-';
    permission_string[6] = (i_mode & 0010) ? 'x' : '-';

    // permissoes de outros
    permission_string[7] = (i_mode & 0004) ? 'r' : '-';
    permission_string[8] = (i_mode & 0002) ? 'w' : '-';
    permission_string[9] = (i_mode & 0001) ? 'x' : '-';

    // finaliza a string
    permission_string[10] = '\0';
}
