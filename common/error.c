#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "sqlite.h"
#include "error.h"

int error_init(error_t *error) {
    int result = strbuf_init(&error->message, 256);
    if (result != SQLITE_OK) {
        return result;
    }
    error->error_count = 0;
    return SQLITE_OK;
}

void error_destroy(error_t *error) {
    if (error == NULL) {
        return;
    }

    strbuf_destroy(&error->message);
}

int error_append_fl(error_t *error, const char* file, int line, const char* msg, ...) {
    if (msg == NULL) {
        printf("Misuse at %s:%d\n", file, line);
        return SQLITE_OK;
    }

    error->error_count++;

    va_list args;
    va_start(args, msg);
    int result = strbuf_vappend(&error->message, msg, args);
    va_end(args);

    if (result == SQLITE_OK) {
        result = strbuf_append(&error->message, "\n");
    }

    return result;
}

size_t error_count(error_t *error) {
    return error->error_count;
}

char* error_message(error_t *error) {
    return strbuf_data_pointer(&error->message);
}