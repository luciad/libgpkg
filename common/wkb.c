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
#include <stdio.h>
#include <string.h>
#include "wkb.h"
#include "sqlite.h"

#define WKB_BE 0
#define WKB_LE 1

#define WKB_XY 0
#define WKB_XYZ 1000
#define WKB_XYM 2000
#define WKB_XYZM 3000

#define WKB_POINT 1
#define WKB_LINESTRING 2
#define WKB_POLYGON 3
#define WKB_MULTIPOINT 4
#define WKB_MULTILINESTRING 5
#define WKB_MULTIPOLYGON 6
#define WKB_GEOMETRYCOLLECTION 7

#define MINMAXCOORD(coord) double coord = coords[off++]; \
  if (coord < env->min_ ## coord) { \
    env->min_ ## coord = coord; \
  } \
  if (coord > env->max_ ## coord) { \
      env->max_ ## coord = coord; \
  }

/** @private */
typedef struct {
    geom_consumer_t consumer;
    geom_envelope_t *header;
} fill_header_t;

static int fill_gpb_envelope_coordinates(const geom_consumer_t *consumer, const geom_header_t *header, size_t point_count, const double *coords) {
    geom_envelope_t *env = ((fill_header_t *) consumer)->header;
    if (header->coord_type == GEOM_XY) {
        env->has_env_x = 1;
        env->has_env_y = 1;
        for (int i = 0, off = 0; i < point_count; i++) {
            MINMAXCOORD(x)
            MINMAXCOORD(y)
        }
    } else if (header->coord_type == GEOM_XYZ) {
        env->has_env_x = 1;
        env->has_env_y = 1;
        env->has_env_z = 1;
        for (int i = 0, off = 0; i < point_count; i++) {
            MINMAXCOORD(x)
            MINMAXCOORD(y)
            MINMAXCOORD(z)
        }
    } else if (header->coord_type == GEOM_XYM) {
        env->has_env_x = 1;
        env->has_env_y = 1;
        env->has_env_m = 1;
        for (int i = 0, off = 0; i < point_count; i++) {
            MINMAXCOORD(x)
            MINMAXCOORD(y)
            MINMAXCOORD(m)
        }
    } else {
        env->has_env_x = 1;
        env->has_env_y = 1;
        env->has_env_z = 1;
        env->has_env_m = 1;
        for (int i = 0, off = 0; i < point_count; i++) {
            MINMAXCOORD(x)
            MINMAXCOORD(y)
            MINMAXCOORD(z)
            MINMAXCOORD(m)
        }
    }
    return SQLITE_OK;
}

int wkb_fill_envelope(binstream_t *stream, geom_envelope_t *envelope) {
    geom_envelope_init(envelope);

    fill_header_t fill_gpb;
    fill_gpb.header = envelope;
    geom_consumer_init(&fill_gpb.consumer, NULL, NULL, NULL, NULL, fill_gpb_envelope_coordinates);
    int result = wkb_read_geometry(stream, &fill_gpb.consumer);

    return result;
}

static int read_wkb_geometry_header(binstream_t *stream, geom_header_t *header) {
    uint8_t order;
    if (binstream_read_u8(stream, &order) != SQLITE_OK) {
        return SQLITE_IOERR;
    }
    binstream_set_endianness(stream, order == WKB_BE ? BIG : LITTLE);

    uint32_t type;
    if (binstream_read_u32(stream, &type) != SQLITE_OK) {
        return SQLITE_IOERR;
    }
    uint32_t modifier = (type / 1000) * 1000;
    type -= modifier * 1000;

    switch (modifier) {
        case WKB_XY:
            header->coord_size = 2;
            header->coord_type = GEOM_XY;
            break;
        case WKB_XYZ:
            header->coord_size = 3;
            header->coord_type = GEOM_XYZ;
            break;
        case WKB_XYM:
            header->coord_size = 3;
            header->coord_type = GEOM_XYM;
            break;
        case WKB_XYZM:
            header->coord_size = 4;
            header->coord_type = GEOM_XYZM;
            break;
        default:
            return SQLITE_IOERR;
    }
    
    switch (type) {
        case WKB_POINT:
            header->geom_type = GEOM_POINT;
            break;
        case WKB_LINESTRING:
            header->geom_type = GEOM_LINESTRING;
            break;
        case WKB_POLYGON:
            header->geom_type = GEOM_POLYGON;
            break;
        case WKB_MULTIPOINT:
            header->geom_type = GEOM_MULTIPOINT;
            break;
        case WKB_MULTILINESTRING:
            header->geom_type = GEOM_MULTILINESTRING;
            break;
        case WKB_MULTIPOLYGON:
            header->geom_type = GEOM_MULTIPOLYGON;
            break;
        case WKB_GEOMETRYCOLLECTION:
            header->geom_type = GEOM_GEOMETRYCOLLECTION;
            break;
        default:
            return SQLITE_IOERR;
    }

    return SQLITE_OK;
}

static int read_point(binstream_t *stream, const geom_consumer_t *consumer, const geom_header_t *header) {
    int result;
    uint32_t coord_size = header->coord_size;
    double coord[coord_size];
    for (int i = 0; i < coord_size; i++) {
        result = binstream_read_double(stream, &coord[i]);
        if (result != SQLITE_OK) {
            return result;
        }
    }

    return consumer->coordinates(consumer, header, 1, coord);
}

#define COORD_BATCH_SIZE 10

static int read_points(binstream_t *stream, const geom_consumer_t *consumer, const geom_header_t *header, uint32_t point_count) {
    int result;
    double coord[header->coord_size * COORD_BATCH_SIZE];

    uint32_t remaining = point_count;
    while(remaining > 0) {
        uint32_t points_to_read = (remaining > COORD_BATCH_SIZE ? COORD_BATCH_SIZE : remaining);
        uint32_t coords_to_read = points_to_read * header->coord_size;
        for (int i = 0; i < coords_to_read; i++) {
            result = binstream_read_double(stream, &coord[i]);
            if (result != SQLITE_OK) {
                return result;
            }
        }

        result = consumer->coordinates(consumer, header, points_to_read, coord);
        if (result != SQLITE_OK) {
            return result;
        }

        remaining -= points_to_read;
    }

    return SQLITE_OK;
}

static int read_linearring(binstream_t *stream, const geom_consumer_t *consumer, const geom_header_t *header) {
    int result = SQLITE_OK;

    uint32_t point_count;
    result = binstream_read_u32(stream, &point_count);
    if (result != SQLITE_OK) {
        goto exit;
    }

    geom_header_t ring_header;
    ring_header.geom_type = GEOM_LINEARRING;
    ring_header.coord_size = header->coord_size;
    ring_header.coord_type = header->coord_type;
    result = consumer->begin_geometry(consumer, &ring_header);
    if (result != SQLITE_OK) {
        goto exit;
    }

    result = read_points(stream, consumer, &ring_header, point_count);
    if (result != SQLITE_OK) {
        goto exit;
    }

    result = consumer->end_geometry(consumer, &ring_header);

  exit:
    return result;
}

static int read_linestring(binstream_t *stream, const geom_consumer_t *consumer, const geom_header_t *header) {
    uint32_t point_count;
    if (binstream_read_u32(stream, &point_count) != SQLITE_OK) {
        return SQLITE_IOERR;
    }

    uint32_t coord_count = header->coord_size * point_count;
    double coord[coord_count];
    for (int i = 0; i < coord_count; i++) {
        if (binstream_read_double(stream, &coord[i]) != SQLITE_OK) {
            return SQLITE_IOERR;
        }
    }
    return consumer->coordinates(consumer, header, point_count, coord);
}

static int read_polygon(binstream_t *stream, const geom_consumer_t *consumer, const geom_header_t *header) {
    uint32_t ring_count;
    if (binstream_read_u32(stream, &ring_count) != SQLITE_OK) {
        return SQLITE_IOERR;
    }

    for (int i = 0; i < ring_count; i++) {
        if (read_linearring(stream, consumer, header) != SQLITE_OK) {
            return SQLITE_IOERR;
        }
    }
    return SQLITE_OK;
}

static int read_geometry(binstream_t *stream, const geom_consumer_t *consumer, const geom_header_t *header);

static int read_multipoint(binstream_t *stream, const geom_consumer_t *consumer, const geom_header_t *header) {
    uint32_t point_count;
    if (binstream_read_u32(stream, &point_count) != SQLITE_OK) {
        return SQLITE_IOERR;
    }

    geom_header_t point_header;
    for (int i = 0; i < point_count; i++) {
        if (read_wkb_geometry_header(stream, &point_header) != SQLITE_OK) {
            return SQLITE_IOERR;
        }

        if (point_header.geom_type != GEOM_POINT || point_header.coord_type != header->coord_type) {
            return SQLITE_IOERR;
        }

        if (read_geometry(stream, consumer, &point_header) != SQLITE_OK) {
            return SQLITE_IOERR;
        }
    }
    return SQLITE_OK;
}

static int read_multilinestring(binstream_t *stream, const geom_consumer_t *consumer, const geom_header_t *header) {
    uint32_t linestring_count;
    if (binstream_read_u32(stream, &linestring_count) != SQLITE_OK) {
        return SQLITE_IOERR;
    }

    geom_header_t linestring_header;
    for (int i = 0; i < linestring_count; i++) {
        if (read_wkb_geometry_header(stream, &linestring_header) != SQLITE_OK) {
            return SQLITE_IOERR;
        }

        if (linestring_header.geom_type != GEOM_LINESTRING || linestring_header.coord_type != header->coord_type) {
            return SQLITE_IOERR;
        }

        if (read_geometry(stream, consumer, &linestring_header) != SQLITE_OK) {
            return SQLITE_IOERR;
        }
    }
    return SQLITE_OK;
}

static int read_multipolygon(binstream_t *stream, const geom_consumer_t *consumer, const geom_header_t *header) {
    uint32_t polygon_count;
    if (binstream_read_u32(stream, &polygon_count) != SQLITE_OK) {
        return SQLITE_IOERR;
    }

    geom_header_t polygon_header;
    for (int i = 0; i < polygon_count; i++) {
        if (read_wkb_geometry_header(stream, &polygon_header) != SQLITE_OK) {
            return SQLITE_IOERR;
        }

        if (polygon_header.geom_type != GEOM_POLYGON || polygon_header.coord_type != header->coord_type) {
            return SQLITE_IOERR;
        }

        if (read_geometry(stream, consumer, &polygon_header) != SQLITE_OK) {
            return SQLITE_IOERR;
        }
    }
    return SQLITE_OK;
}

static int read_geometrycollection(binstream_t *stream, const geom_consumer_t *consumer, const geom_header_t *header) {
    uint32_t geometry_count;
    if (binstream_read_u32(stream, &geometry_count) != SQLITE_OK) {
        return SQLITE_IOERR;
    }

    geom_header_t geometry_header;
    for (int i = 0; i < geometry_count; i++) {
        if (read_wkb_geometry_header(stream, &geometry_header) != SQLITE_OK) {
            return SQLITE_IOERR;
        }

        if (geometry_header.coord_type != header->coord_type) {
            return SQLITE_IOERR;
        }

        if (read_geometry(stream, consumer, &geometry_header) != SQLITE_OK) {
            return SQLITE_IOERR;
        }
    }
    return SQLITE_OK;
}

static int read_geometry(binstream_t *stream, const geom_consumer_t *consumer, const geom_header_t *header) {
    int result = SQLITE_OK;

    int (*read_body)(binstream_t *, const geom_consumer_t *, const geom_header_t *);
    switch (header->geom_type) {
        case GEOM_POINT:
            read_body = read_point;
            break;
        case GEOM_LINESTRING:
            read_body = read_linestring;
            break;
        case GEOM_POLYGON:
            read_body = read_polygon;
            break;
        case GEOM_MULTIPOINT:
            read_body = read_multipoint;
            break;
        case GEOM_MULTILINESTRING:
            read_body = read_multilinestring;
            break;
        case GEOM_MULTIPOLYGON:
            read_body = read_multipolygon;
            break;
        case GEOM_GEOMETRYCOLLECTION:
            read_body = read_geometrycollection;
            break;
        default:
            read_body = NULL;
    }

    if (read_body) {
        result = consumer->begin_geometry(consumer, header);
        if (result != SQLITE_OK) {
            goto exit;
        }

        result = (*read_body)(stream, consumer, header);
        if (result != SQLITE_OK) {
            goto exit;
        }

        result = consumer->end_geometry(consumer, header);
        if (result != SQLITE_OK) {
            goto exit;
        }
    }

  exit:
    return result;
}

static int read_wkb_geometry(binstream_t *stream, const geom_consumer_t *consumer) {
    geom_header_t header;
    int res = read_wkb_geometry_header(stream, &header);
    if (res != SQLITE_OK) {
        return res;
    }

    return read_geometry(stream, consumer, &header);
}

int wkb_read_geometry(binstream_t *stream, const geom_consumer_t *consumer) {
    int result = SQLITE_OK;

    result = consumer->begin(consumer);
    if (result != SQLITE_OK) {
        goto exit;
    }

    result = read_wkb_geometry(stream, consumer);
    if (result != SQLITE_OK) {
        goto exit;
    }

    result = consumer->end(consumer);
    if (result != SQLITE_OK) {
         goto exit;
    }

  exit:
    return result;
}

int wkb_read_header(binstream_t *stream, geom_header_t *header) {
    return read_wkb_geometry_header(stream, header);
}

static int wkb_begin_geometry(const geom_consumer_t *consumer, const geom_header_t *header) {
    int result = SQLITE_OK;

    wkb_writer_t *writer = (wkb_writer_t *) consumer;
    binstream_t *stream = &writer->stream;

    if (writer->offset >= 0) {
        writer->children[writer->offset]++;
    }

    writer->offset++;
    writer->start[writer->offset] = binstream_position(stream);
    writer->children[writer->offset] = 0;

    size_t wkb_header_size;
    switch (header->geom_type) {
        case GEOM_POINT:
            wkb_header_size = 5;
            break;
        case GEOM_LINEARRING:
            wkb_header_size = 4;
            break;
        default:
            wkb_header_size = 9;
    }

    result = binstream_relseek(stream, wkb_header_size);

    return result;
}

static int wkb_coordinates(const geom_consumer_t *consumer, const geom_header_t *header, size_t point_count, const double *coords) {
    int result = SQLITE_OK;

    wkb_writer_t *writer = (wkb_writer_t *) consumer;
    binstream_t *stream = &writer->stream;

    result = binstream_write_ndouble(stream, coords, point_count * header->coord_size);
    if (result != SQLITE_OK) {
        goto exit;
    }

    writer->children[writer->offset]+=point_count;

  exit:
    return result;
}

static int wkb_end_geometry(const geom_consumer_t *consumer, const geom_header_t *header) {
    int result = SQLITE_OK;

    wkb_writer_t *writer = (wkb_writer_t *) consumer;
    binstream_t *stream = &writer->stream;

    size_t current_pos = binstream_position(stream);
    size_t children = writer->children[writer->offset];

    size_t start = writer->start[writer->offset];
    result = binstream_seek(stream, start);
    if (result != SQLITE_OK) {
        goto exit;
    }

    if (header->geom_type == GEOM_LINEARRING) {
        result = binstream_write_u32(stream, (uint32_t)children);
        if (result != SQLITE_OK) {
            goto exit;
        }
    } else {
        uint32_t modifier;
        switch (header->coord_type) {
            default:
            case GEOM_XY:
                modifier = WKB_XY;
                break;
            case GEOM_XYZ:
                modifier = WKB_XYZ;
                break;
            case GEOM_XYM:
                modifier = WKB_XYM;
                break;
            case GEOM_XYZM:
                modifier = WKB_XYZM;
                break;
        }
    
        uint32_t geom_type;
        switch (header->geom_type) {
            default:
            case GEOM_POINT:
                geom_type = WKB_POINT;
                break;
            case GEOM_LINESTRING:
                geom_type = WKB_LINESTRING;
                break;
            case GEOM_POLYGON:
                geom_type = WKB_POLYGON;
                break;
            case GEOM_MULTIPOINT:
                geom_type = WKB_MULTIPOINT;
                break;
            case GEOM_MULTILINESTRING:
                geom_type = WKB_MULTILINESTRING;
                break;
            case GEOM_MULTIPOLYGON:
                geom_type = WKB_MULTIPOLYGON;
                break;
            case GEOM_GEOMETRYCOLLECTION:
                geom_type = WKB_GEOMETRYCOLLECTION;
                break;
        }

        result = binstream_write_u8(stream, binstream_get_endianness(stream) == LITTLE ? WKB_LE : WKB_BE);
        if (result != SQLITE_OK) {
            goto exit;
        }

        result = binstream_write_u32(stream, geom_type + modifier);
        if (result != SQLITE_OK) {
            goto exit;
        }

        if (geom_type != WKB_POINT) {
            result = binstream_write_u32(stream, (uint32_t)children);
            if (result != SQLITE_OK) {
                goto exit;
            }
        }        
    }

    writer->offset--;
    result = binstream_seek(stream, current_pos);

  exit:
    return result;
}

static int wkb_end(const geom_consumer_t *consumer) {
    wkb_writer_t *writer = (wkb_writer_t *) consumer;
    binstream_t *stream = &writer->stream;
    binstream_flip(stream);
    return SQLITE_OK;
}

int wkb_writer_init(wkb_writer_t *writer) {
    geom_consumer_init(&writer->geom_consumer, NULL, wkb_end, wkb_begin_geometry, wkb_end_geometry, wkb_coordinates);
    int res = binstream_init_growable(&writer->stream, 256);
    if (res != SQLITE_OK) {
        return res;
    }

    memset(writer->start, 0, GEOM_MAX_DEPTH * sizeof(size_t));
    memset(writer->children, 0, GEOM_MAX_DEPTH * sizeof(size_t));
    writer->offset = -1;

    return SQLITE_OK;
}

geom_consumer_t* wkb_writer_geom_consumer(wkb_writer_t *writer) {
    return &writer->geom_consumer;
}

void wkb_writer_destroy(wkb_writer_t *writer) {
    binstream_destroy(&writer->stream);
}

uint8_t* wkb_writer_getwkb( wkb_writer_t *writer ) {
    return binstream_data(&writer->stream);
}

size_t wkb_writer_length( wkb_writer_t *writer ) {
    return binstream_available(&writer->stream);
}