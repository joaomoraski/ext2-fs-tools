//
// Created by moraski on 07/06/25.
//

#ifndef EXT2_DEFS_H
#define EXT2_DEFS_H

// Define o maximo de argumentos e tamanho de input
#define BASE_BLOCK 1024 // inicio do superblock
#define IMG_PATH "myext2image.img" // caminho da imagem
#define EXT2_SUPER_MAGIC	0xEF53 // roubei da struct oficial, é o indicador do supermagic
#define EXT2_FT_REG_FILE 1 // indicador de arquivo
#define EXT2_FT_DIR 2 // indicador de diretorio

// mascaras de arquivo, permissao e etc
// usadas para ISOLAR os bits de tipo dos bits de permissão
#define EXT2_S_IFMT   0xF000 // mascara para pegar o tipo

// valores para tipos de arquivos
#define EXT2_S_IFLNK  0xA000 // Link Simbólico
#define EXT2_S_IFREG  0x8000 // Arquivo Regular
#define EXT2_S_IFDIR  0x4000 // Diretório


// mascaras de permissao

// dono (user)
#define EXT2_S_IRUSR  0400
#define EXT2_S_IWUSR  0200
#define EXT2_S_IXUSR  0100

// grupo
#define EXT2_S_IRGRP  0040
#define EXT2_S_IWGRP  0020
#define EXT2_S_IXGRP  0010

// outros
#define EXT2_S_IROTH  0004
#define EXT2_S_IWOTH  0002
#define EXT2_S_IXOTH  0001

#endif //EXT2_DEFS_H
