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
#ifndef GPKG_SPB_H
#define GPKG_SPB_H

#include <stdint.h>
#include "binstream.h"
#include "blobio.h"
#include "geomio.h"
#include "wkb.h"

/**
 * \addtogroup spb Spatialite Binary Header I/O
 * @{
 */

/**
 * Initializes a Spatialite Binary writer.
 * @param writer the writer to initialize
 * @param srid the SRID that should be used
 * @return SQLITE_OK on success, an error code otherwise
 */
int spb_writer_init(geom_blob_writer_t *writer, int32_t srid);

/**
 * Destroys a Spatialite Binary writer.
 * @param writer the writer to destroy
 */
void spb_writer_destroy(geom_blob_writer_t *writer, int free_data);

/**
 * Reads a Spatialite Binary header from the given stream. When this method return SQLITE_OK, the stream is guaranteed
 * to be positioned immediately after the Spatialite Binary header. Otherwise the position is undefined.
 *
 * @param stream the stream to read from
 * @param[out] header the header to populate
 * @param[out] error the error buffer to write to in case of I/O errors
 * @return SQLITE_OK if the header was successfully read\n
 *         SQLITE_IOERR if an I/O error occurred while reading the header
 */
int spb_read_header(binstream_t *stream, geom_blob_header_t *header, errorstream_t *error);

/**
 * Writes a Spatialite Binary header to the given stream. When this method return SQLITE_OK, the stream is guaranteed
 * to be positioned immediately after the Spatialite Binary header. Otherwise the position is undefined.
 *
 * @param stream the stream to write to
 * @param header the header to write to the stream
 * @param[out] error the error buffer to write to in case of I/O errors
 * @return SQLITE_OK if the header was successfully written\n
 *         SQLITE_IOERR if an I/O error occurred while writing the header
 */
int spb_write_header(binstream_t *stream, geom_blob_header_t *header, errorstream_t *error);

/** @} */

#endif
