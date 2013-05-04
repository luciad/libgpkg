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

/**
 * \addtogroup strbuf Strings
 * @{
 */

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

/** @} */

#endif
