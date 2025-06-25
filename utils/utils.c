#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

void format_date(uint32_t timestamp, char *buffer, int buffer_size) {
    time_t brute_time = timestamp;
    struct tm *time_struct = localtime(&brute_time);
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


void print_permissions(uint16_t i_mode) {
    // formato usado como base é o mesmo do ls -l
    // drwxr-xr--
    // d = diretorio, rwx -> dono, r-x -> grupo, r-- -> outros
    // somando 7 = 4 2 1
    // imprimir indicador de diretorio
    if (S_ISDIR(i_mode)) {
        printf("d"); // d para diretorio
    } else if (S_ISLNK(i_mode)) {
        printf("l"); // l para links
    } else {
        printf("-"); // - para arquivos ou outros tipos
    }

    // permissoes do dono
    if (i_mode & S_IRUSR) { printf("r"); } else { printf("-"); }
    if (i_mode & S_IWUSR) { printf("w"); } else { printf("-"); }
    if (i_mode & S_IXUSR) { printf("x"); } else { printf("-"); }

    // permissoes do grupo
    if (i_mode & S_IRGRP) { printf("r"); } else { printf("-"); }
    if (i_mode & S_IWGRP) { printf("w"); } else { printf("-"); }
    if (i_mode & S_IXGRP) { printf("x"); } else { printf("-"); }

    // permissoes do resto
    if (i_mode & S_IROTH) { printf("r"); } else { printf("-"); }
    if (i_mode & S_IWOTH) { printf("w"); } else { printf("-"); }
    if (i_mode & S_IXOTH) { printf("x"); } else { printf("-"); }
}
