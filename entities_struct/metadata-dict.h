#ifndef METADATA_DICT_H
#define METADATA_DICT_H
#include <stddef.h>
#include <stdio.h>

#include "entities.h"


typedef enum {
    FIELD_TYPE_UINT32,
    FIELD_TYPE_STRING,
    FIELD_TYPE_CHAR
} FieldType;

typedef struct {
    const char *name;
    FieldType type;
    size_t offset;
    size_t size;
} FieldMetadata;

const FieldMetadata *find_field_metadata(const char *field_name);

static const FieldMetadata UserRecordMetadata[] = {
    {"id", FIELD_TYPE_UINT32, offsetof(UserRecord, id), sizeof(((UserRecord *) 0)->id)},
    {"is_active", FIELD_TYPE_CHAR, offsetof(UserRecord, is_active), sizeof(((UserRecord *) 0)->is_active)},
    {"username", FIELD_TYPE_STRING, offsetof(UserRecord, username), sizeof(((UserRecord *) 0)->username)},
    {"email", FIELD_TYPE_STRING, offsetof(UserRecord, email), sizeof(((UserRecord *) 0)->email)}
};
static const int UserRecordMetadataSize = sizeof(UserRecordMetadata) / sizeof(FieldMetadata);



#endif
