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
#ifndef GPB_STRBUF_H
#define GPB_STRBUF_H

#include <string.h>
#include <stdarg.h>

/**
 * \addtogroup strbuf Strings
 * @{
 */

/**
 * A string buffer.
 */
typedef struct {
  /** @private */
  char *buffer;
  /** @private */
  size_t capacity;
  /** @private */
  size_t length;
  /** @private */
  int growable;
} strbuf_t;

/**
 * Initializes a string buffer.
 * @param strbuf the string buffer to initialize
 * @param initial_size the initial buffer size to preallocate in bytes
 * @return SQLITE_OK on success, an error code otherwise
 */
int strbuf_init(strbuf_t *strbuf, size_t initial_size);

/**
 * Initializes a fixed size string buffer using the given backing array.
 * @param strbuf the string buffer to initialize
 * @param buffer the data buffer to use
 * @param length the length of buffer in bytes
 * @return SQLITE_OK on success, an error code otherwise
 */
int strbuf_init_fixed(strbuf_t *strbuf, char *buffer, size_t length);

/**
 * Resets a string buffer to the state it had after calling strbuf_init or strbuf_init_fixed.
 * This function erases the content of the data buffer. It does not create a new buffer.
 * @param strbuf the buffer to reset
 * @return SQLITE_OK on success, an error code otherwise
 */
int strbuf_reset(strbuf_t *strbuf);

/**
 * Destroys a string buffer, freeing any internal data structures that have been allocated.
 * @param buffer the string buffer to destroy
 */
void strbuf_destroy(strbuf_t *buffer);

/**
 * Returns the length of the content contained in this string buffer in bytes.
 * @param buffer a string buffer
 * @return the length of the buffer contents in bytes.
 */
size_t strbuf_length(strbuf_t *buffer);

/**
 * Returns a direct pointer to the internal data buffer of a string buffer. Note that subsequent calls
 * to strbuf_append() and strbuf_destroy() may invalidate the return pointer.
 *
 * @param buffer a string buffer
 * @return a direct pointer to the internal data buffer of a string buffer
 */
char *strbuf_data_pointer(strbuf_t *buffer);

/**
 * Returns a copy of the contents of a string buffer.
 *
 * @param buffer a string buffer
 * @param[out] out on success out will be set to point to the allocated copy of the contents
 * @return SQLITE_OK on success, an error code otherwise
 */
int strbuf_data(strbuf_t *buffer, char **out);

/**
 * Appends a formatted string to this string buffer.
 *
 * @param buffer a string buffer
 * @param fmt the string format
 *
 * @return SQLITE_OK on success, an error code otherwise
 */
int strbuf_append(strbuf_t *buffer, const char *fmt, ...);

/**
 * Appends a formatted string to this string buffer.
 *
 * @param buffer a string buffer
 * @param fmt the string format
 * @param args the variable argument list to use as values for the format
 *
 * @return SQLITE_OK on success, an error code otherwise
 */
int strbuf_vappend(strbuf_t *buffer, const char *fmt, va_list args);

/** @} */

#endif
