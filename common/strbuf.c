#include <stdarg.h>
#include <stdio.h>
#include "sqlite.h"
#include "strbuf.h"

int strbuf_init(strbuf_t *buffer, size_t initial_size) {
    void *data = sqlite3_malloc(initial_size);
    if (data == NULL) {
        return GPKG_NOMEM;
    }

    buffer->buffer = data;
    buffer->capacity = initial_size;
    buffer->length = 0;
    return GPKG_OK;
}

void strbuf_destroy(strbuf_t *buffer) {
    sqlite3_free(buffer->buffer);
}

size_t strbuf_length(strbuf_t *buffer) {
    return buffer->length;
}

char * strbuf_data(strbuf_t *buffer) {
    return buffer->buffer;
}

int strbuf_append(strbuf_t *buffer, const char* msg, ...) {
    va_list args;

    va_start(args, msg);
    char* formatted = sqlite3_vmprintf(msg, args);
    va_end(args);
	
	if (formatted == NULL) {
		sqlite3_free(formatted);
		return GPKG_NOMEM;
	}

	size_t formatted_len = strlen(formatted);
	size_t needed_capacity = buffer->length + formatted_len;
	if (needed_capacity > buffer->capacity) {
		size_t new_capacity = buffer->capacity * 3 / 2;
		if (needed_capacity > new_capacity) {
			new_capacity = needed_capacity;
		}
		
		void *data = sqlite3_realloc(buffer->buffer, new_capacity);
		if (data == NULL) {
			return GPKG_NOMEM;
		}

		buffer->buffer = data;
		buffer->capacity = new_capacity;
	}

	memmove(buffer->buffer + buffer->length, formatted, formatted_len);
	buffer->length += formatted_len;

	sqlite3_free(formatted);
	
	return GPKG_OK;
}

