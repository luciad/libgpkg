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
#ifndef GPKG_BINSTREAM_H
#define GPKG_BINSTREAM_H

#include <stdint.h>
#include <stddef.h>

/**
 * \addtogroup binstream Binary I/O
 * @{
 */

typedef enum { LITTLE, BIG } binstream_endianness;

/**
 * A stream-like object that can be used to read/write binary data.
 */
typedef struct {
	uint8_t *data;
	size_t limit;
	size_t position;
    size_t capacity;
	binstream_endianness end;
    int fixed_size;
} binstream_t;

/**
 * Initialise a fixed size binary stream.
 * @param stream the stream to initialize
 * @param data an array of 8-bit bytes that will be encapsulated by the stream
 * @param length the length of data
 */
int binstream_init(binstream_t *stream, uint8_t *data, size_t length);

int binstream_init_growable(binstream_t *stream, size_t initial_cap);

void binstream_destroy(binstream_t *stream);

uint8_t* binstream_data(binstream_t *stream);

size_t binstream_available(binstream_t *stream);

size_t binstream_position(binstream_t *stream);

int binstream_seek(binstream_t *stream, size_t position);

int binstream_relseek(binstream_t *stream, size_t amount);

void binstream_set_endianness(binstream_t *stream, binstream_endianness e);

binstream_endianness binstream_get_endianness(binstream_t *stream);

int binstream_read_u8(binstream_t *stream, uint8_t *out);

int binstream_write_u8(binstream_t *stream, uint8_t val);

int binstream_nread_u8(binstream_t *stream, uint8_t *out, size_t count);

int binstream_write_nu8(binstream_t *stream, uint8_t *val, size_t count);

int binstream_read_u32(binstream_t *stream, uint32_t *out);

int binstream_write_u32(binstream_t *stream, uint32_t val);

int binstream_read_u64(binstream_t *stream, uint64_t *out);

int binstream_write_u64(binstream_t *stream, uint64_t val);

int binstream_read_double(binstream_t *stream, double *out);

int binstream_write_double(binstream_t *stream, double val);

int binstream_write_ndouble(binstream_t *stream, double *val, size_t count);

/** @} */

#endif
