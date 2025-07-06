# Projeto - Implementação do Sistema de Arquivos EXT2

Implementar estruturas de dados e operações para manipular a imagem (.iso) de um sistema de arquivos EXT2. As operações
devem ser invocadas a partir de um prompt (shell). O shell deve executar as operações a partir da referência do
diretório atual. Considere que o programa shell desenvolvido sempre inicia na raiz (/) da imagem manipulada.

## Índice

* [Projeto - Implementação do Sistema de Arquivos EXT2](#projeto---implementação-do-sistema-de-arquivos-ext2)
  * [Índice](#índice)
  * [Inicialização da estrutura](#inicialização-da-estrutura)
    * [Estruturas de Dados Necessárias](#estruturas-de-dados-necessárias)
    * [Comandos](#comandos)
      * [Leitura.](#leitura)
      * [Escrita.](#escrita)
      * [Movimentação para fora da imagem.](#movimentação-para-fora-da-imagem)
  * [Referencias](#referencias)

## Inicialização da estrutura

- [X] Criar estrutura de arquivo
- [X] Criar imagem e montar
- [X] Criar Makefile para gerar a imagem e montar
- [X] Criar esboço do terminal da atividade
    - [X] Usar o da atividade de processos

### Estruturas de Dados Necessárias

- [X] Implementar estrutura do Superblock
- [X] Implementar estrutura do Group Descriptor
- [X] Implementar estrutura do Inode
- [X] Implementar a estrutura da entrada de diretório (dir_entry)
- [X] Implementar a estrutura para manipulação dos dados
    - Estrutura para controlar alguns dados
    - Vai armazenar o superbloco, descritores de grupos, tamanho do bloco
    - número de blocos, atual diretório e o file descriptor da imagem

### Comandos

#### Leitura.

- [X] Comando `info`
    - Exibe informações do disco e do sistema de arquivos.
- [X] Comando `cat <file>`
    - Exibe conteúdo de um arquivo.
- [X] Comando `attr <file | dir>`
    - Mostra atributos do inode(tipo, permissão, timestamp e etc).
- [X] Comando `cd <path>`
    - Navega para diretórios e lista arquivos de um diretório.
- [X] Comando `ls`
    - Lista os arquivos e diretórios do diretório corrente.
    - [X] Melhorar para listar do diretório informado no comando `ls <path>`.
- [X] Comando `pwd`
    - Exibe o caminho absoluto do diretório atual.

#### Escrita.

- [X] Comando `touch <file>`
    - Cria o arquivo _file_ com conteúdo vazio.
    - [X] Melhorar para aceitar múltiplas entradas
- [X] Comando `mkdir <dir>`
    - Cria o diretório _dir_ vazio.
    - [X] Melhorar para aceitar múltiplas entradas
- [X] Comando `rm <file>`
    - Remove o arquivo _file_ do sistema.
    - [X] Melhorar para aceitar múltiplas entradas
- [X] Comando `rmdir <dir>`
    - Remove o diretório _dir_, se estiver vazio.
    - [X] Melhorar para aceitar múltiplas entradas
- [X] Comando `rename <file> <newfilename>`
    - Renomeia o arquivo _file_ para _newfilename_.

#### Movimentação para fora da imagem.

- [X] Comando `cp <source_path> <target_path>`
    - Copia um arquivo de origem (_source_path_) para o destino (_target_path_)
- [X] Comando `mv <source_path> <target_path>`
    - Move um arquivo de origem (_source_path_) para o destino (_target_path_)

## Referencias

[The Second Extended File System | Dave Poirier](https://www.nongnu.org/ext2-doc/ext2.html)<br>
[Design and Implementation of Second Extended Filesystem | Rémy Card, Stephen Tweedie, Theodoro Ts'o](https://e2fsprogs.sourceforge.net/ext2intro.html)<br>
[The Ext2 Filesystem | Emanuele Altieri, Prof, Nicholas Howe](https://www.science.smith.edu/~nhowe/262/oldlabs/ext2.html)<br>
[Ext2 Wikipedia | Wikipedia](https://pt.wikipedia.org/wiki/Ext2)<br>
[C string strtok() function | w3Schools](https://www.w3schools.com/c/ref_string_strtok.php)<br>
[C Pass Addresses and Pointer | Programiz](https://www.programiz.com/c-programming/c-pointer-functions)<br>
[Packed Structures | GNU C](https://www.gnu.org/software/c-intro-and-ref/manual/html_node/Packed-Structures.html)<br>
[Ext2 Tiiks | Michale Meeks](https://github.com/mmeeks/ext2tools)<br>