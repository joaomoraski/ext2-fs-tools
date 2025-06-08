/*
* Autor: João Moraski | RA: 1904000
* Data de criação: 28/05/2025
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/types.h>
#include <ext2fs/ext2fs.h>
#include "ext2-impl/ext2-fs-methods.h"
#include "commands.h"

#define MAX_INPUT 1024
#define MAX_ARGS 100

void print_prompt() {
    // Pegar o nome do usuário
    struct passwd* pw = getpwuid(getuid());
    char* username = pw->pw_name;

    // Trocar para pegar o diretorio da imagem
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));

    // Homenagem ao meu tema do zsh :)
    printf("╭─%s@%s\n╰─$ ", username, cwd);
    fflush(stdout);
}

int main() {
    char input[MAX_INPUT];

    ext2_info fs_info;
    load_super_block(&fs_info);

    while (1) {
        print_prompt();
        // Le a entrada do usuário, caso de algum problema ou ele termine a entrada(Control + D) ele da break na entrada
        if (!fgets(input, sizeof(input), stdin)) { break; }
        input[strcspn(input, "\n")] = 0; // remover \n no final da linha
        if (strlen(input) == 0) continue; // ignorar entradas vazias
        if (strcmp(input, "exit") == 0) { break; } // verifica se é exit, ai sai

        char* args[MAX_ARGS];
        int argc = 0;
        char* token = strtok(input, " "); // split da entrada nos espaços

        while (token != NULL && argc < MAX_ARGS - 1) {
            args[argc++] = token; // adiciona o token para o vetor de args
            token = strtok(NULL, " ");
        }
        args[argc] = NULL;

        if (strcmp(args[0], "help") == 0) {
            printf("Da info ai viado\n");
        }
        else if (strcmp(args[0], "info") == 0) {
            info(fs_info);
        } else if (strcmp(args[0], "ls") == 0) {
            ls(fs_info);
        }
    }
    return 0;
}
