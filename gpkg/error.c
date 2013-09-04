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


int error_init_fixed(error_t *error, char *buffer, size_t length) {
  int result = strbuf_init_fixed(&error->message, buffer, length);
  if (result != SQLITE_OK) {
    return result;
  }
  error->error_count = 0;
  return SQLITE_OK;
}

int error_reset(error_t *error) {
  error->error_count = 0;
  strbuf_reset(&error->message);
}

void error_destroy(error_t *error) {
  if (error == NULL) {
    return;
  }

  strbuf_destroy(&error->message);
}

int error_append(error_t *error, const char *msg, ...) {
  error->error_count++;

  int result = SQLITE_OK;

  if (msg) {
    va_list args;
    va_start(args, msg);
    result = strbuf_vappend(&error->message, msg, args);
    va_end(args);

    if (result == SQLITE_OK) {
      result = strbuf_append(&error->message, "\n");
    }
  }

  return result;
}

size_t error_count(error_t *error) {
  return error->error_count;
}

char *error_message(error_t *error) {
  return strbuf_data_pointer(&error->message);
}
