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
#ifndef GPKG_BLOBIO_H
#define GPKG_BLOBIO_H

#include "geomio.h"
#include "wkb.h"

/**
 * The header of a geometry BLOB as stored in the database.
 */
typedef struct {
  /**
   * The encoding version number.
   */
  uint8_t version;

  /**
   * Indicates if the geometry is empty or not.
   */
  uint8_t empty;

  /**
   * The SRID of the geometry.
   */
  int32_t srid;
  /**
   * The envelope of the geometry.
   */
  geom_envelope_t envelope;
} geom_blob_header_t;

/**
 * A geometry blob writer. geom_blob_writer_t instances can be used to generate a spatial database specific blobs based
 * on any geometry source. Use geom_blob_writer_geom_consumer() to obtain a geom_consumer_t pointer that can be passed to
 * geomtery sources.
 */
typedef struct {
  /** @private */
  geom_consumer_t geom_consumer;
  /** @private */
  geom_blob_header_t header;
  /** @private */
  geom_type_t geom_type;
  /** @private */
  wkb_writer_t wkb_writer;
} geom_blob_writer_t;

/**
 * Returns a geometry blob writer as a geometry consumer. This function should be used
 * to pass the writer to another function that takes a geom_consumer_t as input.
 * @param writer the writer
 */
geom_consumer_t *geom_blob_writer_geom_consumer(geom_blob_writer_t *writer);

/**
 * Returns a pointer to the data that was written by the given writer. The length of the returned
 * buffer can be obtained using the gpb_writer_length() function.
 * @param writer the writer
 * @return a pointer to the data
 */
uint8_t *geom_blob_writer_getdata(geom_blob_writer_t *writer);

/**
 * Returns the length of the buffer obtained using the geom_blob_writer_getdata() function.
 * @param writer the writer
 * @return the length of the data buffer
 */
size_t geom_blob_writer_length(geom_blob_writer_t *writer);

#endif
