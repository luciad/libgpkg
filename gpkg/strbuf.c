/*
 * Copyright 2013 Luciad (http://www.luciad.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdarg.h>
#include <stdio.h>
#include "sqlite.h"
#include "strbuf.h"

int strbuf_init(strbuf_t *strbuf, size_t initial_size) {
  char *data = (char *)sqlite3_malloc((int)initial_size);
  if (data == NULL) {
    return SQLITE_NOMEM;
  }

  strbuf->buffer = data;
  strbuf->capacity = initial_size;
  strbuf->growable = 1;
  strbuf_reset(strbuf);
  return SQLITE_OK;
}

int strbuf_init_fixed(strbuf_t *strbuf, char *buffer, size_t length) {
  strbuf->buffer = buffer;
  strbuf->capacity = length - 1;
  strbuf->growable = 0;
  strbuf_reset(strbuf);
  return SQLITE_OK;
}

int strbuf_reset(strbuf_t *strbuf) {
  memset(strbuf->buffer, 0, strbuf->capacity);
  strbuf->length = 0;
  return SQLITE_OK;
}

void strbuf_destroy(strbuf_t *buffer) {
  if (buffer == NULL) {
    return;
  }

  if (buffer->buffer) {
    if (buffer->growable) {
      sqlite3_free(buffer->buffer);
    }
    buffer->buffer = NULL;
  }
}

size_t strbuf_length(strbuf_t *buffer) {
  return buffer->length;
}

char *strbuf_data_pointer(strbuf_t *buffer) {
  return buffer->buffer;
}

int strbuf_data(strbuf_t *buffer, char **out) {
  size_t length = strbuf_length(buffer);
  *out = (char *)sqlite3_malloc((int)(length + 1));
  if (*out == NULL) {
    return SQLITE_NOMEM;
  } else {
    memmove(*out, buffer->buffer, length);
    (*out)[length] = 0;
    return SQLITE_OK;
  }
}

int strbuf_append(strbuf_t *buffer, const char *msg, ...) {
  int result;
  va_list args;
  va_start(args, msg);
  result = strbuf_vappend(buffer, msg, args);
  va_end(args);
  return result;
}

int strbuf_vappend(strbuf_t *buffer, const char *msg, va_list args) {
  int result = SQLITE_OK;
  char *formatted = sqlite3_vmprintf(msg, args);

  if (formatted == NULL) {
    result = SQLITE_NOMEM;
    goto exit;
  }

  size_t formatted_len = strlen(formatted);
  size_t needed_capacity = buffer->length + formatted_len + 1;
  if (needed_capacity > buffer->capacity) {
    if (buffer->growable) {
      size_t new_capacity = buffer->capacity * 3 / 2;
      if (needed_capacity > new_capacity) {
        new_capacity = needed_capacity;
      }

      char *data = (char *)sqlite3_realloc(buffer->buffer, (int)new_capacity);
      if (data == NULL) {
        result = SQLITE_NOMEM;
        goto exit;
      }

      memset(data + buffer->capacity, 0, new_capacity - buffer->capacity);

      buffer->buffer = data;
      buffer->capacity = new_capacity;
    } else {
      result = SQLITE_NOMEM;
      size_t available = (buffer->capacity - buffer->length);
      if (available > 0) {
        formatted_len = available - 1;
      } else {
        formatted_len = 0;
      }
    }
  }

  if (formatted_len > 0) {
    memmove(buffer->buffer + buffer->length, formatted, formatted_len);
    buffer->length += formatted_len;
    buffer->buffer[buffer->length] = 0;
  }

exit:
  sqlite3_free(formatted);

  return result;
}

