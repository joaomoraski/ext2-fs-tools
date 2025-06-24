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
#include <readline/readline.h>
#include <readline/history.h>
// #include <ext2fs/ext2fs.h> // To usando pra pegar o codigo de exemplo pela IDE
#include "ext2-impl/ext2-fs-methods.h"
#include "commands/commands.h"

#define MAX_INPUT 1024
#define MAX_ARGS 100

void get_prompt_string(ext2_info fs_info, char* prompt_buffer, int buffer_size) {
    struct passwd* pw = getpwuid(getuid());
    char* username = pw->pw_name;

    // Homenagem ao meu tema do zsh :)
    snprintf(prompt_buffer, buffer_size, "╭─%s@(%s)\n╰─$ ", username, fs_info.current_path);
}

int main() {
    char* input;
    char prompt[300];

    ext2_info fs_info;

    int fd;
    if ((fd = open(IMG_PATH, O_RDWR)) < 0) {
        perror(IMG_PATH);
        exit(EXIT_FAILURE);
    }

    fs_info.fd = fd;
    load_super_block(&fs_info);
    load_group_desc(&fs_info);

    while (1) {
        get_prompt_string(fs_info, prompt, sizeof(prompt));

        input = readline(prompt);

        // caso de algum problema ou ele termine a entrada(Control + D) ele da break
        if (!input) {
            printf("\n"); // Para a linha não ficar colada com a saída
            break;
        }

        if (strcmp(input, "exit") == 0) {
            free(input);
            break;
        } // verifica se é exit, ai sai

        if (strlen(input) > 0) {
            add_history(input);
        }

        char* args[MAX_ARGS];
        int argc = 0;
        char* token = strtok(input, " "); // split da entrada nos espaços
        // retorna o ponteiro para o inicio da primeira palavra que encontrou

        while (token != NULL && argc < MAX_ARGS - 1) {
            args[argc++] = token; // adiciona o token para o vetor de args
            token = strtok(NULL, " ");
        }
        args[argc] = NULL;

        if (argc == 0) {
            free(input);
            continue;
        }

        if (strcmp(args[0], "help") == 0) {
            printf("Da info ai viado\n");
        } else if (strcmp(args[0], "clear") == 0) {
            // \033[2J limpa a tela inteira.
            // \033[H move o cursor para o canto.
            printf("\033[2J\033[H");
        } else if (strcmp(args[0], "info") == 0) {
            info(fs_info);
        } else if (strcmp(args[0], "ls") == 0) {
            ls(fs_info);
        } else if (strcmp(args[0], "print") == 0) {
            if (strcmp(args[1], "superblock") == 0) {
                print_superblock(fs_info);
            } else if (strcmp(args[1], "groups") == 0) {
                print_groups(fs_info);
            } else if (strcmp(args[1], "inode") == 0) {
                print_inode(fs_info, atoi(args[2]));
            } else {
                printf("print option not found\n");
            }
        } else if (strcmp(args[0], "pwd") == 0) {
            printf("%s\n", fs_info.current_path);
        } else if (strcmp(args[0], "cd") == 0) {
            cd(&fs_info, args[1]);
        } else {
            printf("command not found\n");
        }

        free(input);
    }
    close(fs_info.fd);
    return 0;
}
