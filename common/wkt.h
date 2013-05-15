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

#include "allocator.h"
#include "binstream.h"
#include "strbuf.h"
#include "geomio.h"

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
} wkt_writer_t;

int wkt_writer_init( wkt_writer_t *writer, allocator_t *allocator );

geom_consumer_t* wkt_writer_geom_consumer(wkt_writer_t *writer);

void wkt_writer_destroy( wkt_writer_t *writer );

char* wkt_writer_getwkt( wkt_writer_t *writer );

size_t wkt_writer_length( wkt_writer_t *writer );

int wkt_read_geometry(char *data, size_t length, geom_consumer_t *consumer);

/** @} */

#endif
