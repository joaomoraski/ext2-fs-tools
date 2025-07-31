#ifndef COMMANDS_H // vai garantir de incluir uma unica vez
#define COMMANDS_H
// definir um espaço de nome
// se tiver mais funçoes com o mesmo nome ele nao vai confundir

#include "../ext2-impl/ext2_structs.h"

void db_select_all(ext2_info* fs_info, char* path, int limit);
void db_insert(ext2_info* fs_info, char* path);

#endif