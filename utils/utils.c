#include "utils.h"

#include <string.h>
#include <time.h>

void format_date(uint32_t timestamp, char* buffer, int buffer_size) {
    time_t brute_time = timestamp;
    struct tm* time_struct = localtime(&brute_time);
    strftime(buffer, buffer_size, "%d/%m/%Y %H:%M", time_struct);
}

void change_path_level(char* current_path) {
    // se ja esta na raiz ok
    if (strcmp(current_path, "/") == 0) {
        return;
    }

    // encontra a posiçao da ultima / no caminho
    char* ultimo_slash = strrchr(current_path, '/');

    // se o ultimo / for o primeiro entao o pai do diretorio é a propria raiz
    // ex /livros
    if (ultimo_slash == current_path) {
        // Colocamos o terminador nulo logo após a primeira '/', transformando "/livros" em "/".
        current_path[1] = '\0';
    } else {
        // ex /livros/classicos
        // adiciona o \0 no fim da string
        *ultimo_slash = '\0';
    }
}
