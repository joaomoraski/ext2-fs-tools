# Projeto - Implementação do Sistema de Arquivos EXT2

Implementar estruturas de dados e operações para manipular a imagem (.iso) de um sistema de arquivos EXT2. As operações devem ser invocadas a partir de um prompt (shell). O shell deve executar as operações a partir da referência do diretório atual. Considere que o programa shell desenvolvido sempre inicia na raiz (/) da imagem manipulada.

instalar  e2fslibs-dev


## Índice
- [Inicialização da estrutura](#inicializacao-da-estrutura)
- [Parte 1](#parte-1)
  - [Comando `info`](#comando-info)
  - [Comando `cat <file>`](#comando-cat-file)
  - [Comando `attr <file | dir>`](#comando-attr-file--dir)
  - [Comando `cd <path>` e `ls`](#comando-cd-path-e-ls)
  - [Comando `pwd`](#comando-pwd)

## Inicialização da estrutura
- [X] Criar estrutura de arquivo
- [X] Criar imagem e montar
- [X] Criar Makefile para gerar a imagem e montar
- [X] Criar esboço do terminal da atividade
    - [X] Usar o da atividade de processos

## Parte 1

### Estruturas de Dados Necessárias
- [ ] Implementar estrutura do Superblock
  - [ ] s_inodes_count: Total de inodes
  - [ ] s_blocks_count: Total de blocos
  - [ ] s_free_blocks_count: Blocos livres
  - [ ] s_free_inodes_count: Inodes livres
  - [ ] s_first_data_block: Primeiro bloco de dados
  - [ ] s_log_block_size: Tamanho do bloco
  - [ ] s_blocks_per_group: Blocos por grupo
  - [ ] s_inodes_per_group: Inodes por grupo

- [ ] Implementar estrutura do Group Descriptor
  - [ ] bg_block_bitmap: Localização do bitmap de blocos
  - [ ] bg_inode_bitmap: Localização do bitmap de inodes
  - [ ] bg_inode_table: Localização da tabela de inodes
  - [ ] bg_free_blocks_count: Blocos livres no grupo
  - [ ] bg_free_inodes_count: Inodes livres no grupo

- [ ] Implementar estrutura do Inode
  - [ ] i_mode: Tipo e permissões
  - [ ] i_uid: ID do usuário
  - [ ] i_size: Tamanho em bytes
  - [ ] i_atime: Último acesso
  - [ ] i_ctime: Criação
  - [ ] i_mtime: Modificação
  - [ ] i_blocks: Número de blocos
  - [ ] i_block[15]: Ponteiros para blocos

### Comandos

- [ ] Comando `info`
    - Exibe informações do disco e do sistema de arquivos.
    - [ ] Ler e exibir dados do Superblock
    - [ ] Mostrar estatísticas do sistema de arquivos
    - [ ] Exibir informações de grupos de blocos

- [ ] Comando `cat <file>`
    - Exibe conteúdo de um arquivo.
    - [ ] Encontrar inode do arquivo
    - [ ] Ler blocos de dados usando i_block[]
    - [ ] Exibir conteúdo em formato texto

- [ ] Comando `attr <file | dir>`
    - Mostra atributos do inode(tipo, permissão, timestamp e etc).
    - [ ] Ler i_mode para tipo e permissões
    - [ ] Exibir timestamps (i_atime, i_ctime, i_mtime)
    - [ ] Mostrar tamanho e blocos usados
    - [ ] Exibir IDs de usuário e grupo

- [ ] Comando `cd <path>` e `ls`
    - Navega para diretorios e lista arquivos de um diretorio.
    - [ ] Implementar navegação em diretórios
    - [ ] Ler entradas de diretório
    - [ ] Exibir lista de arquivos
    - [ ] Manter diretório atual

- [ ] Comando `pwd`
    - Exibe o caminho absoluto do diretorio atual
    - [ ] Manter histórico de navegação
    - [ ] Construir caminho absoluto
    - [ ] Exibir caminho formatado


## Referencias

[Entender sobre o ext2](https://www.nongnu.org/ext2-doc/ext2.html)
[Entender sobre o ext2 Pt2](https://e2fsprogs.sourceforge.net/ext2intro.html)
[Um pouco de implementação](https://www.science.smith.edu/~nhowe/262/oldlabs/ext2.html)