#include "metadata-dict.h"

#include <string.h>


const FieldMetadata *find_field_metadata(const char *field_name) {
    for (int i = 0; i < UserRecordMetadataSize; ++i) {
        if (strcmp(field_name, UserRecordMetadata[i].name) == 0) {
            return &UserRecordMetadata[i];
        }
    }
    return NULL;
}
