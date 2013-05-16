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
#ifndef GPKG_GPB_H
#define GPKG_GPB_H

#include <stdint.h>
#include "binstream.h"
#include "geomio.h"
#include "wkb.h"

/**
 * \addtogroup gpb GeoPackage Binary Header I/O
 * @{
 */

/**
 * A GeoPackage Binary header.
 */
typedef struct {
    /**
     * The GeoPacakge Binary version number.
     */
    uint8_t version;
    /**
     * The SRID of the geometry.
     */
    int32_t srid;
    /**
     * The envelope of the geometry.
     */
    geom_envelope_t envelope;
} gpb_header_t;

/**
 * A GeoPackage Binary writer. gpb_writer_t instances can be used to generate a GeoPackage Binary blob based on
 * any geometry source. Use gpb_writer_geom_consumer() to obtain a geom_consumer_t pointer that can be passed to
 * geomtery sources.
 */
typedef struct {
    /** @private */
    geom_consumer_t geom_consumer;
    /** @private */
    gpb_header_t header;
    /** @private */
    wkb_writer_t wkb_writer;
} gpb_writer_t;

/**
 * Initializes a GeoPackage Binary writer.
 * @param writer the writer to initialize
 * @param srid the SRID that should be used
 * @return SQLITE_OK on success, an error code otherwise
 */
int gpb_writer_init( gpb_writer_t *writer, uint32_t srid );

/**
 * Destroys a GeoPackage Binary writer.
 * @param writer the writer to destroy
 */
void gpb_writer_destroy( gpb_writer_t *writer );

/**
 * Returns a GeoPackage Binary writer as a geometry consumer. This function should be used
 * to pass the writer to another function that takes a geom_consumer_t as input.
 * @param writer the writer
 */
geom_consumer_t * gpb_writer_geom_consumer(gpb_writer_t *writer);

/**
 * Returns a pointer to the GeoPackage Binary data that was written by the given writer. The length of the returned
 * buffer can be obtained using the gpb_writer_length() function.
 * @param writer the writer
 * @return a pointer to the GeoPackage Binary data
 */
uint8_t* gpb_writer_getgpb( gpb_writer_t *writer );

/**
 * Returns the length of the buffer obtained using the gpb_writer_getgpb() function.
 * @param writer the writer
 * @return the length of the GeoPackage Binary data buffer
 */
size_t gpb_writer_length( gpb_writer_t *writer );

/**
 * Reads a GeoPackage Binary header from the given stream. When this method return SQLITE_OK, the stream is guaranteed
 * to be positioned immediately after the GeoPackage Binary header. Otherwise the position is undefined.
 *
 * @param stream the stream to read from
 * @param[out] header the header to populate
 * @return SQLITE_OK if the header was successfully read\n
 *         SQLITE_IOERR if an I/O error occurred while reading the header
 */
int gpb_read_header(binstream_t *stream, gpb_header_t *header, error_t *error);

/**
 * Writes a GeoPackage Binary header to the given stream. When this method return SQLITE_OK, the stream is guaranteed
 * to be positioned immediately after the GeoPackage Binary header. Otherwise the position is undefined.
 *
 * @param stream the stream to write to
 * @param header the header to write to the stream
 * @return SQLITE_OK if the header was successfully written\n
 *         SQLITE_IOERR if an I/O error occurred while writing the header
 */
int gpb_write_header(binstream_t *stream, gpb_header_t *header);

/** @} */

#endif
