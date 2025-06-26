#ifndef UTILS_H
#define UTILS_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void format_date(uint32_t timestamp, char* buffer, int buffer_size);
void change_path_level(char* current_path);
void resolve_path_string(char* resolved_path, const char* current_path, char* path_to_resolve);
void mount_permissions_string(uint16_t i_mode, char* permission_string);
void point_and_write(int fd, long offset, int whence, const void* buffer, size_t bytes_to_write);
bool is_dir(uint16_t i_mode);

#endif
