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
 * @addtogroup binstream Binary I/O
 * @{
 */

/**
 * Enumeration of endianness values.
 */
typedef enum {
  /**
   * Little endian.
   */
  LITTLE,
  /**
   * Big endian.
   */
  BIG
} binstream_endianness;

/**
 * A stream-like object that can be used to read/write binary data.
 */
typedef struct {
  /** @private */
  uint8_t *data;
  /** @private */
  size_t limit;
  /** @private */
  int limit_set;
  /** @private */
  size_t position;
  /** @private */
  size_t capacity;
  /** @private */
  binstream_endianness end;
  /** @private */
  int growable;
} binstream_t;

/**
 * Initialises a fixed size binary stream.
 *
 * @param stream the stream to initialize
 * @param data an array of 8-bit bytes that will be encapsulated by the stream
 * @param length the length of data
 * @return SQLITE_OK if the stream was successfully initialised
 */
int binstream_init(binstream_t *stream, uint8_t *data, size_t length);

/**
 * Initialises a growable size binary stream.
 *
 * @param stream the stream to initialize
 * @param initial_cap the initial buffer capacity to allocate for the stream in bytes
 * @return SQLITE_OK if the stream was successfully initialised.@n
 *         SQLITE_NOMEM if the internal buffer could not be allocated.
 */
int binstream_init_growable(binstream_t *stream, size_t initial_cap);

/**
 * Destroys the given stream.
 *
 * @param stream the stream to destroy
 * @param free_data determines if 0 the internal buffer of the stream will not be freed and should be freed using
                    sqlite3_free later. Otherwise the buffer is freed by this function.
 */
void binstream_destroy(binstream_t *stream, int free_data);

/**
 * Resets the given binary stream. After calling this function the position will be set to 0, the limit will be
 * set to the capacity and the endianness will be reset to LITTLE.
 *
 * @param stream the stream to reset
 */
void binstream_reset(binstream_t *stream);

/**
 * Returns the a pointer to the data buffer of this stream offset by the current position. At most binstream_available()
 * bytes should be read from the returned data buffer.
 *
 * Note that, if the given stream was initialised as growable, subsequent calls to write functions may invalidate the
 * returned pointer.
 *
 * @param stream a stream
 * @return the number of bytes that can be read
 * @see binstream_available
 */
uint8_t *binstream_data(binstream_t *stream);

/**
 * Rewinds the stream, setting the limit to the current position and the position to 0. This function can be used
 * after writing to a stream to prepare it for subsequent read operations.
 * @param stream the stream to flip
 */
void binstream_flip(binstream_t *stream);

/**
 * Returns the number of bytes that can be read from the given stream. This is equivalent to the number of bytes
 * between the position of the stream and the end of the stream.
 * @param stream a stream
 * @return the number of bytes that can be read
 * @see binstream_data
 */
size_t binstream_available(binstream_t *stream);

/**
 * Returns the current position in the given stream.
 * @param stream a stream
 * @return the current position in the stream
 */
size_t binstream_position(binstream_t *stream);

/**
 * Moves the position in this stream to the given absolute position.
 *
 * Seek operations beyond the end of the stream are allowed if the given stream was initialised as growable.
 *
 * @param stream a stream
 * @param position the absolute position to seek to
 * @return SQLITE_OK if the seek operation succeeded.@n
 *         SQLITE_IOERR if a seek beyond the limit of a non-growable stream was requested
 *         SQLITE_NOMEM if a seek beyond the limit of a growable stream was requested, but the internal buffer could not be reallocated
 * @see binstream_relseek
 */
int binstream_seek(binstream_t *stream, size_t position);

/**
 * Moves the position in this stream by the given relative offset.
 *
 * Seek operations beyond the end of the stream are allowed if the given stream was initialised as growable.
 *
 * @param stream a stream
 * @param offset the absolute position to seek to
 * @return SQLITE_OK if the seek operation succeeded.@n
 *         SQLITE_IOERR if a seek to a position smaller than 0 or beyond the limit of a non-growable stream was requested
 *         SQLITE_NOMEM if a seek beyond the limit of a growable stream was requested, but the internal buffer could not be reallocated
 * @see binstream_seek
 */
int binstream_relseek(binstream_t *stream, int32_t offset);

/**
 * Sets the endianness of the given stream. The new endianness will be applied for all subsequent read operations.
 *
 * @param stream a stream
 * @param e the new endianness of the stream.
 */
void binstream_set_endianness(binstream_t *stream, binstream_endianness e);

/**
 * Returns the current endianness of the given stream.
 *
 * @param stream a stream
 * @return the endianness of the stream.
 */
binstream_endianness binstream_get_endianness(binstream_t *stream);

/**
 * Reads a single unsigned 8-bit value from the stream. The position of the stream is advanced by 1.
 * @param stream a stream
 * @param[out] out a memory area to write the read value to.
 * @return SQLITE_OK if the value was read successfully
 *         SQLITE_IOERR if insufficient data is available in the stream
 */
int binstream_read_u8(binstream_t *stream, uint8_t *out);

/**
 * Writes a single unsigned 8-bit value to the stream. The position of the stream is advanced by 1.
 *
 * @param stream a stream
 * @param val the value to write.
 * @return SQLITE_OK if the value was written successfully
 *         SQLITE_IOERR if insufficient space is available in the stream and the stream is not growable
 */
int binstream_write_u8(binstream_t *stream, uint8_t val);

/**
 * Reads count unsigned 8-bit value from the stream. The position of the stream is advanced by count.
 * @param stream a stream
 * @param[out] out a memory area to write the read value to.
 * @param count the number of values to read
 * @return SQLITE_OK if the values were read successfully
 *         SQLITE_IOERR if insufficient data is available in the stream
 */
int binstream_nread_u8(binstream_t *stream, uint8_t *out, size_t count);

/**
 * Writes count unsigned 8-bit value to the stream. The position of the stream is advanced by count.
 *
 * @param stream a stream
 * @param val the values to write.
 * @param count the number of values to write.
 * @return SQLITE_OK if the value was written successfully
 *         SQLITE_IOERR if insufficient space is available in the stream and the stream is not growable
 */
int binstream_write_nu8(binstream_t *stream, const uint8_t *val, size_t count);

/**
 * Reads a single unsigned 32-bit value from the stream. The position of the stream is advanced by 4.
 *
 * @param stream a stream
 * @param[out] out a memory area to write the read value to.
 * @return SQLITE_OK if the value was read successfully
 *         SQLITE_IOERR if insufficient data is available in the stream
 */
int binstream_read_u32(binstream_t *stream, uint32_t *out);

/**
 * Writes a single unsigned 32-bit value to the stream. The position of the stream is advanced by 4.
 *
 * @param stream a stream
 * @param val the value to write.
 * @return SQLITE_OK if the value was written successfully
 *         SQLITE_IOERR if insufficient space is available in the stream and the stream is not growable
 */
int binstream_write_u32(binstream_t *stream, uint32_t val);

/**
 * Reads a single signed 32-bit value from the stream. The position of the stream is advanced by 4.
 *
 * @param stream a stream
 * @param[out] out a memory area to write the read value to.
 * @return SQLITE_OK if the value was read successfully
 *         SQLITE_IOERR if insufficient data is available in the stream
 */
int binstream_read_i32(binstream_t *stream, int32_t *out);

/**
 * Writes a single signed 32-bit value to the stream. The position of the stream is advanced by 4.
 *
 * @param stream a stream
 * @param val the value to write.
 * @return SQLITE_OK if the value was written successfully
 *         SQLITE_IOERR if insufficient space is available in the stream and the stream is not growable
 */
int binstream_write_i32(binstream_t *stream, int32_t val);


/**
 * Reads a single unsigned 64-bit value from the stream. The position of the stream is advanced by 8.
 * @param stream a stream
 * @param[out] out a memory area to write the read value to.
 * @return SQLITE_OK if the value was read successfully
 *         SQLITE_IOERR if insufficient data is available in the stream
 */
int binstream_read_u64(binstream_t *stream, uint64_t *out);

/**
 * Writes a single unsigned 64-bit value to the stream. The position of the stream is advanced by 8.
 *
 * @param stream a stream
 * @param val the value to write.
 * @return SQLITE_OK if the value was written successfully
 *         SQLITE_IOERR if insufficient space is available in the stream and the stream is not growable
 */
int binstream_write_u64(binstream_t *stream, uint64_t val);

/**
 * Reads a single double-precision floating point value from the stream. The position of the stream is advanced by 8.
 *
 * @param stream a stream
 * @param[out] out a memory area to write the read value to.
 * @return SQLITE_OK if the value was read successfully
 *         SQLITE_IOERR if insufficient data is available in the stream
 */
int binstream_read_double(binstream_t *stream, double *out);

/**
 * Writes a single double-precision floating point value to the stream. The position of the stream is advanced by 8.
 *
 * @param stream a stream
 * @param val the value to write.
 * @return SQLITE_OK if the value was written successfully
 *         SQLITE_IOERR if insufficient space is available in the stream and the stream is not growable
 */
int binstream_write_double(binstream_t *stream, double val);

/**
 * Writes count double-precision floating point value to the stream. The position of the stream is advanced by (8 * count).
 *
 * @param stream a stream
 * @param val the values to write.
 * @param count the number of values to write.
 * @return SQLITE_OK if the values were written successfully
 *         SQLITE_IOERR if insufficient space is available in the stream and the stream is not growable
 */
int binstream_write_ndouble(binstream_t *stream, const double *val, size_t count);

/** @} */

#endif
