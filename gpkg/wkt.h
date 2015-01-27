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
#ifndef GPB_WKT_H
#define GPB_WKT_H

#include "binstream.h"
#include "error.h"
#include "geomio.h"
#include "i18n.h"
#include "strbuf.h"

/**
 * \addtogroup wkt Well-known text I/O
 * @{
 */

/**
 * A Well-Known Text writer. wkt_writer_t instances can be used to generate a WKT geometry strings based on
 * any geometry source. Use wkt_writer_geom_consumer() to obtain a geom_consumer_t pointer that can be passed to
 * geomtery sources.
 */
typedef struct {
  /** @private */
  geom_consumer_t geom_consumer;
  /** @private */
  strbuf_t strbuf;
  /** @private */
  int type[GEOM_MAX_DEPTH];
  /** @private */
  int children[GEOM_MAX_DEPTH];
  /** @private */
  int offset;
  /** @private */
  i18n_locale_t *locale;
} wkt_writer_t;

/**
 * Initializes a Well-Known Text writer.
 * @param writer the writer to initialize
 * @return SQLITE_OK on success, an error code otherwise
 */
int wkt_writer_init(wkt_writer_t *writer);

/**
 * Destroys a Well-Known Text writer.
 * @param writer the writer to destroy
 */
void wkt_writer_destroy(wkt_writer_t *writer);

/**
 * Returns a Well-Known Text writer as a geometry consumer. This function should be used
 * to pass the writer to another function that takes a geom_consumer_t as input.
 * @param writer the writer
 */
geom_consumer_t *wkt_writer_geom_consumer(wkt_writer_t *writer);

/**
 * Returns a pointer to the Well-Known Text data that was written by the given writer. The length of the returned
 * buffer can be obtained using the wkt_writer_length() function.
 * @param writer the writer
 * @return a pointer to the Well-Known Text data
 */
char *wkt_writer_getwkt(wkt_writer_t *writer);

/**
 * Returns the length of the buffer obtained using the wkt_writer_getwkt() function.
 * @param writer the writer
 * @return the length of the Well-Known Text data buffer
 */
size_t wkt_writer_length(wkt_writer_t *writer);

/**
 * Parses a Well-Known Text geometry from the given character array.
 *
 * @param data a character array containing WKT geometry
 * @param length the length of data in number of characters
 * @param consumer the geometry consumer that will receive the parsed geometry
 * @param[out] error the error buffer to write to in case of I/O errors
 * @return SQLITE_OK on success, an error code otherwise
 */
int wkt_read_geometry(char const *data, size_t length, geom_consumer_t const *consumer, i18n_locale_t *locale, errorstream_t *error);

/** @} */

#endif
