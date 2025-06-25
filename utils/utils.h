#ifndef UTILS_H
#define UTILS_H
#include <stdint.h>

void format_date(uint32_t timestamp, char* buffer, int buffer_size);
void change_path_level(char* current_path);
void resolve_path_string(char* resolved_path, const char* current_path, char* path_to_resolve);
void print_permissions(uint16_t i_mode);


#endif
