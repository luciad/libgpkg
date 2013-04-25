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
#include <float.h>
#include <string.h>
#include "gpb.h"
#include "sqlite.h"
#include "wkb.h"

#define MINMAXCOORD(coord) double coord = coords[off++]; \
  if (coord < gpb->min_ ## coord) { \
    gpb->min_ ## coord = coord; \
  } \
  if (coord > gpb->max_ ## coord) { \
      gpb->max_ ## coord = coord; \
  }

typedef struct {
    geom_reader_t reader;
    gpb_t *gpb;
} fill_header_t;

static void fill_gpb_envelope_coordinates(geom_reader_t *reader, geom_header_t *header, size_t point_count, double *coords) {
    gpb_t *gpb = ((fill_header_t *) reader)->gpb;
    if (header->coord_type == GEOM_XY) {
        gpb->has_env_x = 1;
        gpb->has_env_y = 1;
        for (int i = 0, off = 0; i < point_count; i++) {
            MINMAXCOORD(x)
            MINMAXCOORD(y)
        }
    } else if (header->coord_type == GEOM_XYZ) {
        gpb->has_env_x = 1;
        gpb->has_env_y = 1;
        gpb->has_env_z = 1;
        for (int i = 0, off = 0; i < point_count; i++) {
            MINMAXCOORD(x)
            MINMAXCOORD(y)
            MINMAXCOORD(z)
        }
    } else if (header->coord_type == GEOM_XYM) {
        gpb->has_env_x = 1;
        gpb->has_env_y = 1;
        gpb->has_env_m = 1;
        for (int i = 0, off = 0; i < point_count; i++) {
            MINMAXCOORD(x)
            MINMAXCOORD(y)
            MINMAXCOORD(m)
        }
    } else {
        gpb->has_env_x = 1;
        gpb->has_env_y = 1;
        gpb->has_env_z = 1;
        gpb->has_env_m = 1;
        for (int i = 0, off = 0; i < point_count; i++) {
            MINMAXCOORD(x)
            MINMAXCOORD(y)
            MINMAXCOORD(z)
            MINMAXCOORD(m)
        }
    }
}

static void gpb_envelope_init(gpb_t *gpb) {
    gpb->has_env_x = 0;
    gpb->min_x = DBL_MAX;
    gpb->max_x = -DBL_MAX;
    gpb->has_env_y = 0;
    gpb->min_y = DBL_MAX;
    gpb->max_y = -DBL_MAX;
    gpb->has_env_z = 0;
    gpb->min_z = DBL_MAX;
    gpb->max_z = -DBL_MAX;
    gpb->has_env_m = 0;
    gpb->min_m = DBL_MAX;
    gpb->max_m = -DBL_MAX;
}

int gpb_envelope_init_from_wkb(binstream_t *stream, gpb_t *gpb) {
    gpb_envelope_init(gpb);

    fill_header_t fill_gpb;
    fill_gpb.gpb = gpb;
    geom_reader_init(&fill_gpb.reader, NULL, NULL, fill_gpb_envelope_coordinates);
    int result = wkb_read_geometry(stream, &fill_gpb.reader);

    return result;
}

size_t gpb_size(gpb_t *gpb) {
    return (size_t)
            4 // Magic number
            + 4 // SRID
            + 8 * ((gpb->has_env_x ? 2 : 0) + (gpb->has_env_y ? 2 : 0) + (gpb->has_env_z ? 2 : 0) + (gpb->has_env_m ? 2 : 0)); // Envelope
}

int gpb_read_header(binstream_t *stream, gpb_t *gpb) {
    uint8_t head[3];
    if (binstream_nread_u8(stream, head, 3)) {
        return 1;
    }

    if (memcmp(head, "GPB", 3) != 0) {
//        sqlite3_result_error(context, "Incorrect magic number", -1);
        return 1;
    }

    uint8_t flags;
    if (binstream_read_u8(stream, &flags)) {
//        sqlite3_result_error(context, "Could not read flags; insufficient bytes", -1);
        return 1;
    }

    gpb->version = (flags & 0xF0) >> 4;
    uint8_t envelope = (flags & 0xE) >> 1;
    uint8_t endian = flags & 0x1;

    if (gpb->version != 1) {
//        sqlite3_result_error(context, "Incorrect version number", -1);
        return 1;
    }

    if (envelope > 4) {
//        sqlite3_result_error(context, "Invalid envelope content value", -1);
        return 1;
    }

    binstream_set_endianness(stream, endian == 0 ? BIG : LITTLE);

    if (binstream_read_u32(stream, &gpb->srid)) {
//        sqlite3_result_error(context, "Could not read srid; insufficient bytes", -1);
        return 1;
    }

    if (envelope > 0) {
        gpb->has_env_x = 1;
        if (binstream_read_double(stream, &gpb->min_x)) {
//            sqlite3_result_error(context, "Could not read min x; insufficient bytes", -1);
            return 1;
        }
        if (binstream_read_double(stream, &gpb->max_x)) {
//            sqlite3_result_error(context, "Could not read max x; insufficient bytes", -1);
            return 1;
        }
        gpb->has_env_y = 1;
        if (binstream_read_double(stream, &gpb->min_y)) {
//            sqlite3_result_error(context, "Could not read min y; insufficient bytes", -1);
            return 1;
        }
        if (binstream_read_double(stream, &gpb->max_y)) {
//            sqlite3_result_error(context, "Could not read max y; insufficient bytes", -1);
            return 1;
        }
    } else {
        gpb->has_env_x = 0;
        gpb->min_x = 0.0;
        gpb->max_x = 0.0;
        gpb->has_env_y = 0;
        gpb->min_y = 0.0;
        gpb->max_y = 0.0;
    }

    if (envelope == 2 || envelope == 4) {
        gpb->has_env_z = 1;
        if (binstream_read_double(stream, &gpb->min_z)) {
//            sqlite3_result_error(context, "Could not read min z; insufficient bytes", -1);
            return 1;
        }
        if (binstream_read_double(stream, &gpb->max_z)) {
//            sqlite3_result_error(context, "Could not read max z; insufficient bytes", -1);
            return 1;
        }
    } else {
        gpb->has_env_z = 0;
        gpb->min_z = 0.0;
        gpb->max_z = 0.0;
    }

    if (envelope == 3 || envelope == 4) {
        gpb->has_env_m = 1;
        if (binstream_read_double(stream, &gpb->min_m)) {
//            sqlite3_result_error(context, "Could not read min m; insufficient bytes", -1);
            return 1;
        }
        if (binstream_read_double(stream, &gpb->max_m)) {
//            sqlite3_result_error(context, "Could not read max m; insufficient bytes", -1);
            return 1;
        }
    } else {
        gpb->has_env_m = 0;
        gpb->min_m = 0.0;
        gpb->max_m = 0.0;
    }

    return SQLITE_OK;
}

int gpb_write_header(binstream_t *stream, gpb_t *gpb) {
    if (binstream_write_nu8(stream, (uint8_t*)"GPB", 3)) {
//        sqlite3_result_error(context, "Could not write header; insufficient space", -1);
        return 1;
    }

    uint8_t version = gpb->version;
    uint8_t envelope = 0;
    if (gpb->has_env_x && gpb->has_env_y) {
        envelope = 1;
        if (gpb->has_env_z && gpb->has_env_m) {
            envelope = 4;
        } else if (gpb->has_env_z) {
            envelope = 2;
        } else if (gpb->has_env_m) {
            envelope = 3;
        }
    }
    uint8_t endian = binstream_get_endianness(stream) == LITTLE ? 1 : 0;

    uint8_t flags = (version << 4) | (envelope << 1) | endian;
    if (binstream_write_u8(stream, flags)) {
//        sqlite3_result_error(context, "Could not write flags; insufficient space", -1);
        return 1;
    }

    if (binstream_write_u32(stream, gpb->srid)) {
//        sqlite3_result_error(context, "Could not write srid; insufficient space", -1);
        return 1;
    }

    if (gpb->has_env_x) {
        if (binstream_write_double(stream, gpb->min_x)) {
//            sqlite3_result_error(context, "Could not write min x; insufficient space", -1);
            return 1;
        }
        if (binstream_write_double(stream, gpb->max_x)) {
//            sqlite3_result_error(context, "Could not write max x; insufficient space", -1);
            return 1;
        }
    }

    if (gpb->has_env_y) {
        if (binstream_write_double(stream, gpb->min_y)) {
//            sqlite3_result_error(context, "Could not write min y; insufficient space", -1);
            return 1;
        }
        if (binstream_write_double(stream, gpb->max_y)) {
//            sqlite3_result_error(context, "Could not write max y; insufficient space", -1);
            return 1;
        }
    }

    if (gpb->has_env_z) {
        if (binstream_write_double(stream, gpb->min_z)) {
//            sqlite3_result_error(context, "Could not write min z; insufficient space", -1);
            return 1;
        }
        if (binstream_write_double(stream, gpb->max_z)) {
//            sqlite3_result_error(context, "Could not write max z; insufficient space", -1);
            return 1;
        }
    }

    if (gpb->has_env_m) {
        if (binstream_write_double(stream, gpb->min_m)) {
//            sqlite3_result_error(context, "Could not write min m; insufficient space", -1);
            return 1;
        }
        if (binstream_write_double(stream, gpb->max_m)) {
//            sqlite3_result_error(context, "Could not write max m; insufficient space", -1);
            return 1;
        }
    }

    return SQLITE_OK;
}

static void gpb_begin(geom_reader_t *reader, geom_header_t *header) {
    gpb_writer_t *writer = (gpb_writer_t *) reader;
    binstream_t *stream = &writer->stream;

    if (writer->offset >= 0) {
        writer->children[writer->offset]++;
    } else {
        gpb_t *gpb_header = &writer->gpb;
        if (header->geom_type != GEOM_POINT) {
            switch(header->coord_type) {
                case GEOM_XYZ:
                    gpb_header->has_env_x = 1;
                    gpb_header->has_env_y = 1;
                    gpb_header->has_env_z = 1;
                    break;
                case GEOM_XYM:
                    gpb_header->has_env_x = 1;
                    gpb_header->has_env_y = 1;
                    gpb_header->has_env_m = 1;
                    break;
                case GEOM_XYZM:
                    gpb_header->has_env_x = 1;
                    gpb_header->has_env_y = 1;
                    gpb_header->has_env_z = 1;
                    gpb_header->has_env_m = 1;
                    break;
                default:
                    gpb_header->has_env_x = 1;
                    gpb_header->has_env_y = 1;
                    break;
            }
        }
        binstream_relseek(stream, gpb_size(gpb_header));
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
    binstream_relseek(stream, wkb_header_size);
}

static void gpb_coordinates(geom_reader_t *reader, geom_header_t *header, size_t point_count, double *coords) {
    gpb_writer_t *writer = (gpb_writer_t *) reader;
    binstream_t *stream = &writer->stream;

    binstream_write_ndouble(stream, coords, point_count * header->coord_size);

    gpb_t *gpb = &writer->gpb;
    int offset = 0;
    switch(header->coord_type) {
        #define MIN_MAX(coord) double coord = coords[offset++]; \
        if (coord < gpb->min_##coord) gpb->min_##coord = coord; \
        if (coord > gpb->max_##coord) gpb->max_##coord = coord;
        case GEOM_XYZ:
            for(int i = 0; i < point_count; i++) {
                MIN_MAX(x)
                MIN_MAX(y)
                MIN_MAX(z)
            }
            break;
        case GEOM_XYM:
            for(int i = 0; i < point_count; i++) {
                MIN_MAX(x)
                MIN_MAX(y)
                MIN_MAX(m)
            }
            break;
        case GEOM_XYZM:
            for(int i = 0; i < point_count; i++) {
                MIN_MAX(x)
                MIN_MAX(y)
                MIN_MAX(z)
                MIN_MAX(m)
            }
            break;
        default:
            for(int i = 0; i < point_count; i++) {
                MIN_MAX(x)
                MIN_MAX(y)
            }
            break;
        #undef MIN_MAX
    }

    writer->children[writer->offset]+=point_count;
}

static void gpb_end(geom_reader_t *reader, geom_header_t *header) {
    gpb_writer_t *writer = (gpb_writer_t *) reader;
    binstream_t *stream = &writer->stream;

    size_t current_pos = binstream_position(stream);
    size_t children = writer->children[writer->offset];

    binstream_seek(stream, writer->start[writer->offset]);

    switch (header->geom_type) {
        case GEOM_POINT:
            binstream_write_u8(stream, binstream_get_endianness(stream) == LITTLE ? GEOM_LE : GEOM_BE);
            binstream_write_u32(stream, header->geom_type + header->coord_type);
            break;
        case GEOM_LINEARRING:
            binstream_write_u32(stream, (uint32_t)children);
            break;
        default:
            binstream_write_u8(stream, binstream_get_endianness(stream) == LITTLE ? GEOM_LE : GEOM_BE);
            binstream_write_u32(stream, header->geom_type + header->coord_type);
            binstream_write_u32(stream, (uint32_t)children);
    }

    binstream_seek(stream, current_pos);

    writer->offset--;

    if (writer->offset < 0) {
        binstream_seek(stream, 0);
        gpb_write_header(stream, &writer->gpb);
        binstream_seek(stream, 0);
    }
}

int gpb_writer_init( gpb_writer_t *writer, uint32_t srid ) {
    geom_reader_init(&writer->geom_reader, gpb_begin, gpb_end, gpb_coordinates);
    int res = binstream_init_growable(&writer->stream, 256);
    if (res != SQLITE_OK) {
        return res;
    }

    gpb_envelope_init(&writer->gpb);
    writer->gpb.version = 1;
    writer->gpb.srid = srid;

    memset(writer->start, 0, GEOM_MAX_DEPTH * sizeof(size_t));
    memset(writer->children, 0, GEOM_MAX_DEPTH * sizeof(size_t));
    writer->offset = -1;

    return SQLITE_OK;
}

void gpb_writer_destroy( gpb_writer_t *writer ) {
    binstream_destroy(&writer->stream);
}

uint8_t* gpb_writer_getgpb( gpb_writer_t *writer ) {
    return binstream_data(&writer->stream);
}

size_t gpb_writer_length( gpb_writer_t *writer ) {
    return binstream_available(&writer->stream);
}
