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
#include <signal.h>
#include <sys/types.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "ext2-impl/ext2-fs-methods.h"
#include "commands/commands.h"
#include "utils/utils.h"

#define HISTORY_FILE ".ext2_shell_history"
#define MAX_INPUT 1024
#define MAX_ARGS 100

void get_prompt_string(ext2_info fs_info, char* prompt_buffer, int buffer_size) {
    struct passwd* pw = getpwuid(getuid());
    char* username = pw->pw_name;

    // Homenagem ao meu tema do zsh :)
    snprintf(prompt_buffer, buffer_size, "╭─%s@(%s)\n╰─$ ", username, fs_info.current_path);
}

void save_history_on_shutdown() {
    write_history(HISTORY_FILE);
    exit(EXIT_SUCCESS);
}

int main() {
    // se for parado por ctrl+c escreve o historico de comandos tambem
    // antes era so pelo exit
    signal(SIGINT, save_history_on_shutdown);
    signal(SIGTERM, save_history_on_shutdown);
    // lendo o historico antigo da lib history.
    read_history(".ext2_shell_history");

    // começa a carregar as informações do ext2
    // superblock e group desc
    ext2_info fs_info;
    int fd;
    if ((fd = open(IMG_PATH, O_RDWR)) < 0) {
        perror(IMG_PATH);
        exit(EXIT_FAILURE);
    }

    fs_info.fd = fd;
    load_super_block(&fs_info);
    load_group_desc(&fs_info);

    // inicializa a parte de ler comandos
    char prompt[300];
    while (1) {
        get_prompt_string(fs_info, prompt, sizeof(prompt));

        char* input = readline(prompt);

        // caso de algum problema ou ele termine a entrada(Control + D) ele da break
        if (!input) {
            printf("\n"); // para a linha não ficar colada com a saida
            break;
        }

        // verifica se é exit, ai sai
        if (strcmp(input, "exit") == 0) {
            free(input);
            break;
        }

        // se o tamanho for maior que 0 ele salvas no historico
        if (strlen(input) > 0) {
            add_history(input);
        }

        char* args[MAX_ARGS];
        // novo parse do input
        int argc = parse_input(input, args);

        // se não foi setado nenhum arg ele limpa a entrada e continua pro proximo
        if (argc == 0) {
            free(input);
            continue;
        }

        // todo implementar o help de vdd
        if (strcmp(args[0], "help") == 0) {
            printf("Da info ai viado\n");
        } else if (strcmp(args[0], "clear") == 0) {
            // \033[2J limpa a tela inteira.
            // \033[H move o cursor para o canto.
            printf("\033[2J\033[H");
        } else if (strcmp(args[0], "info") == 0) {
            info(fs_info);
        } else if (strcmp(args[0], "ls") == 0) {
            if (args[1] != NULL) {
                ls(fs_info, args[1]);
            } else {
                ls(fs_info, NULL);
            }
        } else if (strcmp(args[0], "print") == 0) {
            if (strcmp(args[1], "superblock") == 0) {
                print_superblock(fs_info);
            } else if (strcmp(args[1], "groups") == 0) {
                print_groups(fs_info);
            } else if (strcmp(args[1], "inode") == 0) {
                if (args[2] == NULL) {
                    printf("print inode: falta operando.\n");
                    continue;
                }
                print_inode(fs_info, atoi(args[2]));
            } else {
                printf("print option not found\n");
            }
        } else if (strcmp(args[0], "pwd") == 0) {
            printf("%s\n", fs_info.current_path);
        } else if (strcmp(args[0], "cd") == 0) {
            if (args[1] != NULL) {
                cd(&fs_info, args[1]);
            } else {
                printf("cd: falta operando.\n");
            }
        } else if (strcmp(args[0], "attr") == 0) {
            if (args[1] != NULL) {
                attr(&fs_info, args[1]);
            } else {
                printf("attr: falta operando.\n");
            }
        } else if (strcmp(args[0], "cat") == 0) {
            if (args[1] != NULL) {
                cat(&fs_info, args[1]);
            } else {
                printf("cat: falta operando.\n");
            }
        } else if (strcmp(args[0], "touch") == 0) {
            if (argc == 2) {
                touch(&fs_info, args[1]);
            } else if (argc > 2) {
                multi_touch(&fs_info, args, argc);
            } else {
                printf("touch: falta operando.\n");
            }
        } else if (strcmp(args[0], "mkdir") == 0) {
            if (argc == 2) {
                cmd_mkdir(&fs_info, args[1]);
            } else if (argc > 2) {
                multi_cmd_mkdir(&fs_info, args, argc);
            } else {
                printf("mkdir: falta operando.\n");
            }
        } else if (strcmp(args[0], "rm") == 0) {
            if (argc == 2) {
                rm(&fs_info, args[1]);
            } else if (argc > 2) {
                multi_rm(&fs_info, args, argc);
            } else {
                printf("rm: falta operando.\n");
            }
        } else if (strcmp(args[0], "rmdir") == 0) {
            if (argc == 2) {
                cmd_rmdir(&fs_info, args[1]);
            } else if (argc > 2) {
                multi_cmd_rmdir(&fs_info, args, argc);
            } else {
                printf("rmdir: falta operando.\n");
            }
        } else if (strcmp(args[0], "cp") == 0) {
            if (args[2] != NULL) {
                cp(&fs_info, args[1], args[2]);
            } else {
                printf("cp: falta operando.\n");
            }
        } else if (strcmp(args[0], "mv") == 0) {
            if (args[2] != NULL) {
                mv(&fs_info, args[1], args[2]);
            } else {
                printf("mv: falta operando.\n");
            }
        } else {
            printf("command not found: %s\n", args[0]);
        }

        free(input);
    }
    write_history(".ext2_shell_history");
    close(fs_info.fd);
    return 0;
}
