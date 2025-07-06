# Projeto de Sistemas Operacionais - Shell para EXT2

Este projeto consiste em uma implementação de um shell de linha de comando em C, desenvolvido como atividade para a
disciplina de Sistemas Operacionais. A aplicação é capaz de ler e manipular uma imagem de disco no formato EXT2,
permitindo a execução de operações fundamentais como navegação, leitura, criação e remoção de arquivos e diretórios.

O shell utiliza a biblioteca `GNU Readline` para prover uma interface de usuário rica, com histórico de comandos e
edição de linha, e implementa do zero toda a lógica para interagir com as estruturas de dados do EXT2,
como o Superbloco, Descritores de Grupo, Tabela de Inodes e Bitmaps.

---
* [Projeto de Sistemas Operacionais - Shell para EXT2](#projeto-de-sistemas-operacionais---shell-para-ext2)
  * [Funcionalidades Implementadas](#funcionalidades-implementadas)
  * [Estrutura do Projeto](#estrutura-do-projeto)
  * [Como Compilar e Executar](#como-compilar-e-executar)
  * [Testes e Validação](#testes-e-validação)
  * [Decisões de Design e Desafios](#decisões-de-design-e-desafios)
  * [Autor](#autor)
  * [Licença](#licença)
---

## Funcionalidades Implementadas

A tabela a seguir detalha todos os comandos implementados no shell:

| Comando  | Sintaxe                    | Descrição                                                                                                  |
|:---------|:---------------------------|:-----------------------------------------------------------------------------------------------------------|
| `info`   | `info`                     | Exibe informações gerais do sistema de arquivos, como contadores de inodes e blocos.                       |
| `ls`     | `ls [path]`                | Lista os arquivos e diretórios do diretório corrente ou do `path` especificado.                            |
| `cd`     | `cd <path>`                | Altera o diretório corrente para o `path` especificado. Suporta `.` e `..`.                                |
| `pwd`    | `pwd`                      | Exibe o caminho absoluto do diretório corrente.                                                            |
| `attr`   | `attr <path>`              | Exibe os atributos do inode do arquivo ou diretório especificado.                                          |
| `cat`    | `cat <path>`               | Exibe o conteúdo de um arquivo. Suporta blocos diretos e indiretos.                                        |
| `touch`  | `touch <file1> [file2...]` | Cria um ou mais arquivos vazios.                                                                           |
| `mkdir`  | `mkdir <dir1> [dir2...]`   | Cria um ou mais diretórios vazios.                                                                         |
| `rm`     | `rm <file1> [file2...]`    | Remove um ou mais arquivos. Lida com a contabilidade de `i_links_count`.                                   |
| `rmdir`  | `rmdir <dir1> [dir2...]`   | Remove um ou mais diretórios, somente se estiverem vazios.                                                 |
| `rename` | `rename <old> <new>`       | Renomeia um arquivo ou diretório dentro do mesmo diretório pai.                                            |
| `cp`     | `cp <src_img> <dest_host>` | Copia um arquivo de dentro da imagem para o sistema de arquivos do hospedeiro.                             |
| `mv`     | `mv <src_img> <dest_host>` | Move um arquivo de dentro da imagem para o sistema de arquivos do hospedeiro (executa `cp` e depois `rm`). |
| `clear`  | `clear`                    | Limpa a tela do terminal.                                                                                  |
| `exit`   | `exit`                     | Encerra a execução do shell.                                                                               |

---

## Estrutura do Projeto

O código-fonte está organizado nos seguintes diretórios:

```
/
  ├── commands/         # Contém a implementação de cada comando do shell (ls.c, cd.c, etc.)
    ├── commands.c
    ├── commands.h
  ├── ext2-impl/        # Lógica principal para interagir com o EXT2
  │   ├── ext2-fs-methods.c
  │   └── ext2\_structs.h
  ├── utils/            # Funções de ajuda genéricas (parser de input, formatadores, e etc.)
    ├── utils.c
    ├── utils.h
  ├── relatorio_tecnico_ext2_1904000.pdf # O relatório técnico do projeto
  ├── Demais arquivos
  └── Makefile          # Arquivo para compilação e verificação e etc
````
---

## Como Compilar e Executar

O projeto utiliza um `Makefile` para automatizar o processo.
**Para compilar o shell:**
Este comando ira compilar o shell
```bash
make
```


**Para executar o shell:**
Este comando irá compilar (se necessário) e depois iniciar o shell, passando a imagem de disco padrão como argumento.

```bash
make run
```

**Para limpar os arquivos compilados:**
Este comando remove o executável e os arquivos objeto.

```bash
make clean
```

-----

## Testes e Validação

A imagem pode ser validada com a ferramenta padrão `e2fsck`.

**Para rodar a verificação de integridade:**

```bash
make verify-ext2
```

Um ciclo de teste completo envolve a criação e remoção de arquivos e diretórios, seguido pela execução de
`make verify-ext2` para garantir que nenhuma inconsistência (como "inodes órfãos" ou erros de bitmap) foi introduzida.

-----

## Decisões de Design e Desafios

Durante o desenvolvimento, algumas decisões de engenharia foram tomadas para gerenciar a complexidade e garantir a
robustez:

* **Parser de Comandos:** Foi implementado um parser manual para a linha de comando que lida com argumentos contendo
  espaços através de aspas, uma vez que `strtok` se mostrou insuficiente.
* **Alocação Genérica:** As lógicas para alocação/desalocação de inodes e blocos, que são muito similares, foram
  abstraídas em funções genéricas `allocate_item` e `deallocate_item` para evitar duplicação de código.
* **Gerenciamento de Entradas de Diretório:** A função `add_dir_entry` implementa uma estratégia para
  reutilizar espaços deixados por entradas deletadas, otimizando o uso do espaço no bloco de diretório.

-----

## Autor

* **João Vitor Moraski Lunkes** - [RA: 1904000]

-----

## Licença

Este projeto é distribuído sob a licença MIT. Veja o arquivo `LICENSE` para mais detalhes.
