#ifndef ENTITIES_H
#define ENTITIES_H
#include <stdint.h>

#define USERNAME_MAX_LEN 27
#define EMAIL_MAX_LEN 31

typedef struct __attribute__((packed)) {
    uint32_t id; // 4 bytes
    uint8_t is_active; // 1 byte
    char username[USERNAME_MAX_LEN]; // 27 bytes
    char email[EMAIL_MAX_LEN]; // 31 bytes
    char padding[1]; // 1 byte
    // Total = 4 + 1 + 27 + 31 + 1 = 64 bytes.
} UserRecord;

#endif //ENTITIES_H
