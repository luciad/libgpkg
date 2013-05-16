#ifndef ERROR_H
#define ERROR_H

#include "strbuf.h"

typedef struct {
    strbuf_t message;
    size_t error_count;
} error_t;

int error_init(error_t *error);

void error_destroy(error_t *error);

int error_append_fl(error_t *error, const char* file, int line, const char* msg, ...);

#define error_append(error, msg...) error_append_fl( error, __FILE__, __LINE__, msg )

size_t error_count(error_t *error);

char* error_message(error_t *error);

#endif