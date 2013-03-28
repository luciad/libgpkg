#ifndef GPB_STRBUF_H
#define GPB_STRBUF_H

#include <string.h>

typedef struct {
    char *buffer;
    size_t capacity;
    size_t length;
} strbuf_t;

int strbuf_init(strbuf_t *buffer, size_t initial_size);

void strbuf_destroy(strbuf_t *buffer);

size_t strbuf_length(strbuf_t *buffer);

char *strbuf_data_pointer(strbuf_t *buffer);

int strbuf_data(strbuf_t *buffer, char **out);

int strbuf_append(strbuf_t *buffer, const char* msg, ...);

#endif
