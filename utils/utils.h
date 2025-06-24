#ifndef UTILS_H
#define UTILS_H
#include <stdint.h>

void format_date(uint32_t timestamp, char* buffer, int buffer_size);
void change_path_level(char* current_path);

#endif
