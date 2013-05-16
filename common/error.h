#ifndef ERROR_H
#define ERROR_H

#include "strbuf.h"

typedef struct {
    strbuf_t message;
    size_t error_count;
} error_t;

int error_init(error_t *error);

int error_init_fixed(error_t *error, char* buffer, size_t length);

void error_destroy(error_t *error);

int error_append(error_t *error, const char* msg, ...);

size_t error_count(error_t *error);

char* error_message(error_t *error);

#endif