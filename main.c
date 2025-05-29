/*
* Autor: João Moraski | RA: 1904000
* Data de criação: 28/05/2025
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/types.h>
#include <ext2fs/ext2fs.h>
#include <ext2fs/ext2_fs.h>


// Define o maximo de argumentos e tamanho de input
#define MAX_INPUT 1024
#define BASE_BLOCK 1024 // inicio do superblock
#define IMG_PATH "myext2.iso" // caminho da imagem
#define MAX_ARGS 100

// tem que extrair daqui
static unsigned int block_size = 0; // vai ser calculado ainda

void print_prompt() {
	// Pegar o nome do usuário
	struct passwd *pw = getpwuid(getuid());
	char *username = pw->pw_name;

	// Trocar para pegar o diretorio da imagem
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));

	// Homenagem ao meu tema do zsh :)
	printf("╭─%s@%s\n╰─$ ", username, cwd);
	fflush(stdout);
}

int main() {
	char input[MAX_INPUT];;

	while (1) {
		print_prompt();

		// Le a entrada do usuário, caso de algum problema ou ele termine a entrada(Control + D) ele da break na entrada
		if (!fgets(input, sizeof(input), stdin)) {
			break;
		}

		input[strcspn(input, "\n")] = 0; // remover \n no final da linha

		if (strlen(input) == 0) continue;  // ignorar entradas vazias

		if (strcmp(input, "exit") == 0) { // verifica se é exit, ai sai
			break;
		}

		char *args[MAX_ARGS];
		int argc = 0;
		char *token = strtok(input, " ");  // split da entrada nos espaços
		int background = 0;

		while (token != NULL && argc < MAX_ARGS - 1) {
			if (strcmp(token, "&") == 0) { // se for backgroound sinaliza para o codigo
				background = 1;
			} else {
				args[argc++] = token; // se não for, adiciona o token para o vetor de args
			}
			// Split pelo espaço em branco
			token = strtok(NULL, " ");
		}
		// Passa null no final pela necessidade do execv
		args[argc] = NULL;

		// Printa o comando
		// for (int i = 0; i < argc; i++) {
		// 	printf("[%s] %s\n", input, args[i]);
		// }

		while(!strcmp("info",input)) {
			// tem que extrair daqui
			struct ext2_super_block super_block;
			int fd;

			if ((fd = open(IMG_PATH, O_RDONLY)) < 0) {
				perror(IMG_PATH);
				return EXIT_FAILURE;
			}

			unsigned char buffer[512];
			lseek(fd, BASE_BLOCK, SEEK_SET);
			read(fd, &super_block, sizeof(super_block));
			close(fd);

			if (super_block.s_magic != EXT2_SUPER_MAGIC) {
				fprintf(stderr, "Not an ext2fs filesystem\n");
				return EXIT_FAILURE;
			}
			block_size = 1024 << super_block.s_log_block_size;

			printf("Reading super-block from device " IMG_PATH ":\n"
				   "Inodes count            : %u\n"
				   "Blocks count            : %u\n"
				   "Reserved blocks count   : %u\n"
				   "Free blocks count       : %u\n"
				   "Free inodes count       : %u\n"
				   "First data block        : %u\n"
				   "Block size              : %u\n"
				   "Blocks per group        : %u\n"
				   "Inodes per group        : %u\n"
				   "Creator OS              : %u\n"
				   "First non-reserved inode: %u\n"
				   "Size of inode structure : %hu\n"
				   ,
				   super_block.s_inodes_count,
				   super_block.s_blocks_count,
				   super_block.s_r_blocks_count,     /* reserved blocks count */
				   super_block.s_free_blocks_count,
				   super_block.s_free_inodes_count,
				   super_block.s_first_data_block,
				   block_size,
				   super_block.s_blocks_per_group,
				   super_block.s_inodes_per_group,
				   super_block.s_creator_os,
				   super_block.s_first_ino,          /* first non-reserved inode */
				   super_block.s_inode_size);
			break;
		}





		// pid_t pid = fork();

		// if (pid == 0) {
		// 	if (background) {
		// 		// remover a saída padrão e erro para /dev/null se for para background
		// 		// A ideia aq é impedir que apareça no terminal o que ta me background
		// 		// Foi mal professor, mas essa eu pedi do chatgpt mesmo, nao consegui fazer sozinho
		// 		int null_fd = open("/dev/null", O_WRONLY);
		// 		dup2(null_fd, STDOUT_FILENO);
		// 		dup2(null_fd, STDERR_FILENO);
		// 		close(null_fd);
		// 	}
		// 	// Executa o comando
		// 	execvp(args[0], args); // Caso nao de errado ele assume o papel do filho e executa o comando e finaliza
		// 	// Como o execvp substitui o filho, caso de erro na execução ele retorna como 1 para indicar erro ao pai
		// 	exit(1);
		// } else if (pid > 0) { // Se for o pai
		// 	if (!background) { // Se n for backgroound espera aexecução terminar
		// 		waitpid(pid, NULL, 0);
		// 	} else { // Se for background ele apenas print o que esta rodando e o pid
		// 		printf("[Rodando em background - PID %d]\n", pid);
		// 	}
		// }
	}
	return 0;
}