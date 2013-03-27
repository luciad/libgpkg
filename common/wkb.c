#include "wkb.h"
#include "sqlite.h"

static int read_wkb_geometry_header(binstream_t *stream, geom_header_t *header) {
    uint8_t order;
    if (binstream_read_u8(stream, &order) != SQLITE_OK) {
//        reader->errmsg = "Could not read byte order";
        return SQLITE_IOERR;
    }
    binstream_set_endianness(stream, order == GEOM_BE ? BIG : LITTLE);

    uint32_t type;
    if (binstream_read_u32(stream, &type) != SQLITE_OK) {
//        reader->errmsg = "Could not read geometry type";
        return SQLITE_IOERR;
    }
    uint32_t modifier = (type / 1000) * 1000;
    type -= modifier * 1000;

    switch (modifier) {
        case GEOM_XY:
            header->coord_size = 2;
            header->coord_type = GEOM_XY;
            break;
        case GEOM_XYZ:
            header->coord_size = 3;
            header->coord_type = GEOM_XYZ;
            break;
        case GEOM_XYM:
            header->coord_size = 3;
            header->coord_type = GEOM_XYM;
            break;
        case GEOM_XYZM:
            header->coord_size = 4;
            header->coord_type = GEOM_XYZM;
            break;
        default:
//            reader->errmsg = "Invalid coordinate type";
            return SQLITE_IOERR;
    }
    header->geom_type = type;

    return SQLITE_OK;
}

static int read_point(binstream_t *stream, geom_reader_t *reader, geom_header_t *header) {
    uint32_t coord_size = header->coord_size;
    double coord[coord_size];
    for (int i = 0; i < coord_size; i++) {
        if (binstream_read_double(stream, &coord[i]) != SQLITE_OK) {
//            reader->errmsg = "Could not read point coordinates";
            return SQLITE_IOERR;
        }
    }
    if (reader->coordinates) {
        reader->coordinates(reader, header, 1, coord);
    }
    return SQLITE_OK;
}

static int read_linearring(binstream_t *stream, geom_reader_t *reader, geom_header_t *header) {
    uint32_t point_count;
    if (binstream_read_u32(stream, &point_count) != SQLITE_OK) {
//        reader->errmsg = "Could not read linear ring point count";
        return SQLITE_IOERR;
    }

    geom_header_t ring_header;
    ring_header.geom_type = GEOM_LINEARRING;
    ring_header.coord_size = header->coord_size;
    ring_header.coord_type = header->coord_type;
    if (reader->begin) {
        reader->begin(reader, &ring_header);
    }

    uint32_t coord_count = header->coord_size * point_count;
    double coord[coord_count];
    for (int i = 0; i < coord_count; i++) {
        if (binstream_read_double(stream, &coord[i]) != SQLITE_OK) {
//            reader->errmsg = "Could not read linear ring point";
            return SQLITE_IOERR;
        }
    }
    if (reader->coordinates) {
        reader->coordinates(reader, header, point_count, coord);
    }

    if (reader->end) {
        reader->end(reader, &ring_header);
    }

    return SQLITE_OK;
}

static int read_linestring(binstream_t *stream, geom_reader_t *reader, geom_header_t *header) {
    uint32_t point_count;
    if (binstream_read_u32(stream, &point_count) != SQLITE_OK) {
//        reader->errmsg = "Could not read linestring point count";
        return SQLITE_IOERR;
    }

    uint32_t coord_count = header->coord_size * point_count;
    double coord[coord_count];
    for (int i = 0; i < coord_count; i++) {
        if (binstream_read_double(stream, &coord[i]) != SQLITE_OK) {
//            reader->errmsg = "Could not read linestring point";
            return SQLITE_IOERR;
        }
    }
    reader->coordinates(reader, header, point_count, coord);
    return SQLITE_OK;
}

static int read_polygon(binstream_t *stream, geom_reader_t *reader, geom_header_t *header) {
    uint32_t ring_count;
    if (binstream_read_u32(stream, &ring_count) != SQLITE_OK) {
//        reader->errmsg = "Could not read polygon ring count";
        return SQLITE_IOERR;
    }

    for (int i = 0; i < ring_count; i++) {
        if (read_linearring(stream, reader, header) != SQLITE_OK) {
//            reader->errmsg = "Could not read polygon ring";
            return SQLITE_IOERR;
        }
    }
    return SQLITE_OK;
}

static int read_geometry(binstream_t *stream, geom_reader_t *reader, geom_header_t *header);

static int read_multipoint(binstream_t *stream, geom_reader_t *reader, geom_header_t *header) {
    uint32_t point_count;
    if (binstream_read_u32(stream, &point_count) != SQLITE_OK) {
//        reader->errmsg = "Could not read multipoint point count";
        return SQLITE_IOERR;
    }

    geom_header_t point_header;
    for (int i = 0; i < point_count; i++) {
        if (read_wkb_geometry_header(stream, &point_header) != SQLITE_OK) {
            return SQLITE_IOERR;
        }

        if (point_header.geom_type != GEOM_POINT || point_header.coord_type != header->coord_type) {
//            reader->errmsg = "Multipoint contains non-point sub geometry";
            return SQLITE_IOERR;
        }

        if (read_geometry(stream, reader, &point_header) != SQLITE_OK) {
            return SQLITE_IOERR;
        }
    }
    return SQLITE_OK;
}

static int read_multilinestring(binstream_t *stream, geom_reader_t *reader, geom_header_t *header) {
    uint32_t linestring_count;
    if (binstream_read_u32(stream, &linestring_count) != SQLITE_OK) {
//        reader->errmsg = "Could not read multilinestring linestring count";
        return SQLITE_IOERR;
    }

    geom_header_t linestring_header;
    for (int i = 0; i < linestring_count; i++) {
        if (read_wkb_geometry_header(stream, &linestring_header) != SQLITE_OK) {
            return SQLITE_IOERR;
        }

        if (linestring_header.geom_type != GEOM_LINESTRING || linestring_header.coord_type != header->coord_type) {
//            reader->errmsg = "Multilinestring contains non-linestring sub geometry";
            return SQLITE_IOERR;
        }

        if (read_geometry(stream, reader, &linestring_header) != SQLITE_OK) {
            return SQLITE_IOERR;
        }
    }
    return SQLITE_OK;
}

static int read_multipolygon(binstream_t *stream, geom_reader_t *reader, geom_header_t *header) {
    uint32_t polygon_count;
    if (binstream_read_u32(stream, &polygon_count) != SQLITE_OK) {
//        reader->errmsg = "Could not read multipolygon polygon count";
        return SQLITE_IOERR;
    }

    geom_header_t polygon_header;
    for (int i = 0; i < polygon_count; i++) {
        if (read_wkb_geometry_header(stream, &polygon_header) != SQLITE_OK) {
            return SQLITE_IOERR;
        }

        if (polygon_header.geom_type != GEOM_POLYGON || polygon_header.coord_type != header->coord_type) {
//            reader->errmsg = "Multipolygon contains non-polygon sub geometry";
            return SQLITE_IOERR;
        }

        if (read_geometry(stream, reader, &polygon_header) != SQLITE_OK) {
            return SQLITE_IOERR;
        }
    }
    return SQLITE_OK;
}

static int read_geometrycollection(binstream_t *stream, geom_reader_t *reader, geom_header_t *header) {
    uint32_t geometry_count;
    if (binstream_read_u32(stream, &geometry_count) != SQLITE_OK) {
//        reader->errmsg = "Could not read geometry collection geometry count";
        return SQLITE_IOERR;
    }

    geom_header_t geometry_header;
    for (int i = 0; i < geometry_count; i++) {
        if (read_wkb_geometry_header(stream, &geometry_header) != SQLITE_OK) {
            return SQLITE_IOERR;
        }

        if (geometry_header.coord_type != header->coord_type) {
//            reader->errmsg = "Geometry collection contains geometry with different coordinate type";
            return SQLITE_IOERR;
        }

        if (read_geometry(stream, reader, &geometry_header) != SQLITE_OK) {
            return SQLITE_IOERR;
        }
    }
    return SQLITE_OK;
}

static int read_geometry(binstream_t *stream, geom_reader_t *reader, geom_header_t *header) {
    int (*read_body)(binstream_t *, geom_reader_t *, geom_header_t *);
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
//            reader->errmsg = "Invalid geometry type";
            read_body = NULL;
    }

    if (read_body) {
        if (reader->begin) {
            reader->begin(reader, header);
        }
        int result = (*read_body)(stream, reader, header);
        if (result == SQLITE_OK) {
            if (reader->end) {
                reader->end(reader, header);
            }
        }
        return result;
    } else {
        return SQLITE_IOERR;
    }
}

static int read_wkb_geometry(binstream_t *stream, geom_reader_t *reader) {
    geom_header_t header;
    int res = read_wkb_geometry_header(stream, &header);
    if (res != SQLITE_OK) {
        return res;
    }

    return read_geometry(stream, reader, &header);
}

int wkb_read_geometry(binstream_t *stream, geom_reader_t *reader) {
    return read_wkb_geometry(stream, reader);
}

int wkb_read_header(binstream_t *stream, geom_header_t *header) {
    return read_wkb_geometry_header(stream, header);
}