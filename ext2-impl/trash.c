//
// Created by magaluraski on 25/06/25.
//

// apenas um monte de codigo que eu refatorei e nao quero perder a versao inicial.


// // retorna o numero do inode alocado ou 0 em caso de erro
// unsigned int find_and_alocate_free_inode(ext2_info* fs_info) {
//     // loop para cada grupo no sistema de arquivos
//     for (int i = 0; i < fs_info->num_block_groups; ++i) {
//         // bitmap esta dentro do descritor de grupo
//         group_desc gd = fs_info->group_desc_array[i];
//
//         // Se o grupo nao tiver inodes, continua
//         if (gd.bg_free_inodes_count == 0) {
//             continue;
//         }
//
//         unsigned int inode_bitmap_block_num = gd.bg_inode_bitmap;
//         char bitmap_buffer[fs_info->block_size];
//         read_data_block(fs_info, inode_bitmap_block_num, bitmap_buffer, sizeof(bitmap_buffer));
//
//         // loop para cada BYTE do bitmap
//         for (int byte_index = 0; byte_index < fs_info->block_size; ++byte_index) {
//             // verifica se o bit foi setado ou nao
//
//             // loop para cada BIT dentro do byte atual (8 bits por byte)
//             for (int bit_idx = 0; bit_idx < 8; bit_idx++) {
//                 // verifica se o bit foi setado ou nao
//                 if (!(bitmap_buffer[byte_index] & (1 << (bit_idx)))) {
//                     // bit livre
//                     // alocar o inode
//                     // calcular o numero do inode baseado na posição do bitmap
//                     // soma os inodes dos grupos anteriores + os bits dos bytes anteriores + o bit atual + 1 para achar o ID.
//                     unsigned int inode_number = (i * fs_info->sb.s_inodes_per_group) + (byte_index * 8) + (bit_idx +
//                         1);
//
//                     // me lembro de fazer isso em microcontroladores
//                     bitmap_buffer[byte_index] |= (1 << (bit_idx)); // setar bit como 1(usado)
//
//                     // salvar o bitmap
//                     point_and_write(fs_info->fd, inode_bitmap_block_num * fs_info->block_size, SEEK_SET,
//                                     bitmap_buffer, fs_info->block_size);
//
//                     // alterar os contadores de inodes free
//                     fs_info->sb.s_free_inodes_count--;
//                     fs_info->group_desc_array[i].bg_free_inodes_count--;
//
//                     // salvar no disco as informações
//                     point_and_write(fs_info->fd, 1024, SEEK_SET, &fs_info->sb, sizeof(super_block));
//
//                     // salvar o descritor de grupo
//                     // calcular a posiçao exata em bytes do descritor de grupo
//                     // block size * 2 é pq a tabela esta no segundo bloco do sistema
//                     // pula i group descs para asber qual modificar
//                     unsigned int group_desc_position = fs_info->block_size * 2 + (i * sizeof(group_desc));
//                     point_and_write(fs_info->fd, group_desc_position, SEEK_SET,
//                                     &(fs_info->group_desc_array[i]), sizeof(group_desc));
//
//                     return inode_number;
//                 }
//             }
//         }
//     }
//
//     printf("Erro: Sistema sem inodes livres\n");
//     return 0;
// }
//
// // retorna o numero do bloco alocado ou 0 em caso de erro
// unsigned int find_and_alocate_free_block(ext2_info* fs_info) {
//     // loop para cada grupo no sistema de arquivos
//     for (int i = 0; i < fs_info->num_block_groups; ++i) {
//         // bitmap esta dentro do descritor de grupo
//         group_desc gd = fs_info->group_desc_array[i];
//
//         // Se o grupo nao tiver blocos, continua
//         if (gd.bg_free_blocks_count == 0) {
//             continue;
//         }
//
//         unsigned int block_bitmap_block_num = gd.bg_block_bitmap;
//         char bitmap_buffer[fs_info->block_size];
//         read_data_block(fs_info, block_bitmap_block_num, bitmap_buffer, sizeof(bitmap_buffer));
//
//         // loop para cada BYTE do bitmap
//         for (int byte_index = 0; byte_index < fs_info->block_size; ++byte_index) {
//             // verifica se o bit foi setado ou nao
//
//             // loop para cada BIT dentro do byte atual (8 bits por byte)
//             for (int bit_idx = 0; bit_idx < 8; bit_idx++) {
//                 if (!(bitmap_buffer[byte_index] & (1 << (bit_idx)))) {
//                     // bit livre
//                     // alocar o bloco
//                     // calcular o numero do bloco baseado na posição do bitmap
//                     // soma os blocos dos grupos anteriores + os bits dos bytes anteriores + o bit atual + 1 para achar o ID.
//                     unsigned int block_number = (i * fs_info->sb.s_blocks_per_group) + (byte_index * 8) + (bit_idx +
//                         1);
//
//                     // me lembro de fazer isso em microcontroladores
//                     bitmap_buffer[byte_index] |= (1 << (bit_idx)); // setar bit como 1(usado)
//
//                     // salvar o bitmap
//                     point_and_write(fs_info->fd, block_bitmap_block_num * fs_info->block_size, SEEK_SET,
//                                     bitmap_buffer, fs_info->block_size);
//
//                     // alterar os contadores de blocos free
//                     fs_info->sb.s_free_blocks_count--;
//                     fs_info->group_desc_array[i].bg_free_blocks_count--;
//
//                     // salvar no disco as informações
//                     point_and_write(fs_info->fd, 1024, SEEK_SET, &fs_info->sb, sizeof(super_block));
//
//                     // salvar o descritor de grupo
//                     // calcular a posiçao exata em bytes do descritor de grupo
//                     // block size * 2 é pq a tabela esta no segundo bloco do sistema
//                     // pula i group descs para asber qual modificar
//                     unsigned int group_desc_position = fs_info->block_size * 2 + (i * sizeof(group_desc));
//                     point_and_write(fs_info->fd, group_desc_position, SEEK_SET,
//                                     &(fs_info->group_desc_array[i]), sizeof(group_desc));
//
//                     return block_number;
//                 }
//             }
//         }
//     }
//
//     printf("Erro: Sistema sem blocos livres\n");
//     return 0;
// }
