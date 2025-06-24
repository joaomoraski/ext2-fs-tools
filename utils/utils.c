#include "utils.h"

#include <time.h>

void format_date(uint32_t timestamp, char* buffer, int buffer_size) {
    time_t brute_time = timestamp;
    struct tm* time_struct = localtime(&brute_time);
    strftime(buffer, buffer_size, "%d/%m/%Y %H:%M", time_struct);
}
