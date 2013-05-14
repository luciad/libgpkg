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
#ifndef GPKG_WKB_H
#define GPKG_WKB_H

#include "binstream.h"
#include "geomio.h"

/**
 * \addtogroup wkb Well-known binary I/O
 * @{
 */

/**
 * A Well-Known Binary writer. wkb_writer_t instances can be used to generate a WKB blob based on
 * any geometry source. Use wkb_writer_geom_consumer() to obtain a geom_consumer_t pointer that can be passed to
 * geomtery sources.
 */
typedef struct {
    /** @private */
    geom_consumer_t geom_consumer;
    /** @private */
    binstream_t stream;
    /** @private */
    size_t start[GEOM_MAX_DEPTH];
    /** @private */
    size_t children[GEOM_MAX_DEPTH];
    /** @private */
    int offset;
} wkb_writer_t;

int wkb_writer_init( wkb_writer_t *writer );

geom_consumer_t* wkb_writer_geom_consumer(wkb_writer_t *writer);

void wkb_writer_destroy( wkb_writer_t *writer );

uint8_t* wkb_writer_getwkb( wkb_writer_t *writer );

size_t wkb_writer_length( wkb_writer_t *writer );

int wkb_read_geometry(binstream_t *stream, geom_consumer_t *consumer);

int wkb_read_header(binstream_t *stream, geom_header_t *header);

int wkb_fill_envelope(binstream_t *stream, geom_envelope_t *envelope);

/** @} */

#endif
