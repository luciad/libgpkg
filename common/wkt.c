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
#include <stdlib.h>
#include <stdio.h>
#include "str.h"
#include "sqlite.h"
#include "wkt.h"
#include "error.h"

static int wkt_begin_geometry(const geom_consumer_t *consumer, const geom_header_t *header) {
    int result = SQLITE_OK;

    wkt_writer_t *writer = (wkt_writer_t *) consumer;

    if (writer->offset >= 0) {
        if (writer->children[writer->offset] > 0) {
            result = strbuf_append(&writer->strbuf, ", ");
        } else {
            result = strbuf_append(&writer->strbuf, "(");
        }
        writer->children[writer->offset]++;
    }

    if (result != SQLITE_OK) {
        goto exit;
    }

    writer->offset++;
    writer->type[writer->offset] = header->geom_type;
    writer->children[writer->offset] = 0;

    if (writer->offset == 0 || writer->type[writer->offset - 1] == GEOM_GEOMETRYCOLLECTION) {
        switch (header->geom_type) {
            case GEOM_POINT:
                result = strbuf_append(&writer->strbuf, "Point ");
                break;
            case GEOM_LINESTRING:
                result = strbuf_append(&writer->strbuf, "LineString ");
                break;
            case GEOM_POLYGON:
                result = strbuf_append(&writer->strbuf, "Polygon ");
                break;
            case GEOM_MULTIPOINT:
                result = strbuf_append(&writer->strbuf, "MultiPoint ");
                break;
            case GEOM_MULTILINESTRING:
                result = strbuf_append(&writer->strbuf, "MultiLineString ");
                break;
            case GEOM_MULTIPOLYGON:
                result = strbuf_append(&writer->strbuf, "MultiPolygon ");
                break;
            case GEOM_GEOMETRYCOLLECTION:
                result = strbuf_append(&writer->strbuf, "GeometryCollection ");
                break;
            case GEOM_LINEARRING:
                // Should never happen, since linear ring is not a top level geometry type.
                result = strbuf_append(&writer->strbuf, "LinearRing ");
                break;
        }

        if (result != SQLITE_OK) {
            goto exit;
        }

        if (header->coord_type == GEOM_XYZ) {
            result = strbuf_append(&writer->strbuf, "Z ");
        } else if (header->coord_type == GEOM_XYM) {
            result = strbuf_append(&writer->strbuf, "M ");
        } else if (header->coord_type == GEOM_XYZM) {
            result = strbuf_append(&writer->strbuf, "ZM ");
        }

        if (result != SQLITE_OK) {
            goto exit;
        }
    }

  exit:
    return result;
}

#define WKT_COORD "%.25g"
#define WKT_COORD_2 WKT_COORD " " WKT_COORD
#define WKT_COORD_3 WKT_COORD_2 " " WKT_COORD
#define WKT_COORD_4 WKT_COORD_3 " " WKT_COORD

static int wkt_coordinates(const geom_consumer_t *consumer, const geom_header_t *header, size_t point_count, const double *coords) {
    int result = SQLITE_OK;

    wkt_writer_t *writer = (wkt_writer_t *) consumer;

    if (writer->children[writer->offset] == 0) {
        result = strbuf_append(&writer->strbuf, "(");
    }
    writer->children[writer->offset]++;

    if (result != SQLITE_OK) {
        goto exit;
    }

    int offset = 0;
    if (header->coord_size == 2) {
        for (int i = 0; i < point_count; i++) {
			      double x = coords[offset++];
			      double y = coords[offset++];

            if (i == 0) {
                result = strbuf_append(&writer->strbuf, WKT_COORD_2, x, y);
            } else {
                result = strbuf_append(&writer->strbuf, ", "WKT_COORD_2, x, y);
            }

            if (result != SQLITE_OK) {
                goto exit;
            }
        }
    } else if (header->coord_size == 3) {
        for (int i = 0; i < point_count; i++) {
			      double x = coords[offset++];
			      double y = coords[offset++];
			      double zm = coords[offset++];
            if (i == 0) {
                result = strbuf_append(&writer->strbuf, WKT_COORD_3, x, y, zm);
            } else {
                result = strbuf_append(&writer->strbuf, ", "WKT_COORD_3, x, y, zm);
            }

            if (result != SQLITE_OK) {
                goto exit;
            }
        }
    } else if (header->coord_size == 4) {
        for (int i = 0; i < point_count; i++) {
			      double x = coords[offset++];
			      double y = coords[offset++];
			      double z = coords[offset++];
			      double m = coords[offset++];
            if (i == 0) {
                result = strbuf_append(&writer->strbuf, WKT_COORD_4, x, y, z, m);
            } else {
                result = strbuf_append(&writer->strbuf, ", "WKT_COORD_4, x, y, z, m);
            }

            if (result != SQLITE_OK) {
                goto exit;
            }
        }
    }

  exit:
    return result;
}

static int wkt_end_geometry(const geom_consumer_t *consumer, const geom_header_t *header) {
    int result = SQLITE_OK;

    wkt_writer_t *writer = (wkt_writer_t *) consumer;

    if (writer->children[writer->offset] == 0) {
        result = strbuf_append(&writer->strbuf, "EMPTY");
    } else {
        result = strbuf_append(&writer->strbuf, ")");
    }
    writer->offset--;

    return result;
}

int wkt_writer_init(wkt_writer_t *writer) {
    geom_consumer_init(&writer->geom_consumer, NULL, NULL, wkt_begin_geometry, wkt_end_geometry, wkt_coordinates);
    int res = strbuf_init(&writer->strbuf, 256);
    if (res != SQLITE_OK) {
        return res;
    }

    memset(writer->type, 0, GEOM_MAX_DEPTH);
    memset(writer->children, 0, GEOM_MAX_DEPTH);
    writer->offset = -1;

    return SQLITE_OK;
}

geom_consumer_t* wkt_writer_geom_consumer(wkt_writer_t *writer) {
    return &writer->geom_consumer;
}

void wkt_writer_destroy(wkt_writer_t *writer) {
    strbuf_destroy(&writer->strbuf);
}

char *wkt_writer_getwkt(wkt_writer_t *writer) {
    return strbuf_data_pointer(&writer->strbuf);
}

size_t wkt_writer_length(wkt_writer_t *writer) {
    return strbuf_length(&writer->strbuf);
}

/** @private */
typedef enum {
    WKT_POINT,
    WKT_POLYGON,
    WKT_LINESTRING,
    WKT_MULTIPOINT,
    WKT_MULTIPOLYGON,
    WKT_MULTILINESTRING,
    WKT_GEOMETRYCOLLECTION,
    WKT_Z,
    WKT_M,
    WKT_ZM,
    WKT_EMPTY,
    WKT_LPAREN,
    WKT_RPAREN,
    WKT_COMMA,
    WKT_NUMBER,
    WKT_EOF,
    WKT_ERROR
} wkt_token;

/** @private */
typedef struct {
    const char *start;
    const char *end;

    wkt_token token;
    double token_value;
} wkt_tokenizer_t;

static void wkt_next_token(wkt_tokenizer_t *tok) {
    const char *start = tok->start;
    const char *end = tok->end;
    while (start < end) {
        char c = *start;
        while (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            start++;
            if (start == end) {
                goto eof;
            }
            c = *start;
        }

        if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z')) {
            const char *tok_end = start;
            do {
                tok_end++;
                if (tok_end == end) {
                    break;
                } else {
                    c = *tok_end;
                }
            } while (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'));
            size_t token_length = tok_end - start;
            tok->start = tok_end;

#define CHECK_TOKEN(token_name) if (STRNICMP( #token_name, start, token_length ) == 0) { tok->token = WKT_##token_name; }
#define CHECK_TOKEN_1(tok1) CHECK_TOKEN(tok1) else { goto error; }
#define CHECK_TOKEN_2(tok1, tok2) CHECK_TOKEN(tok1) else CHECK_TOKEN_1(tok2)
            if (token_length == 1) {
                CHECK_TOKEN_2(Z, M)
            } else if (token_length == 2) {
                CHECK_TOKEN_1(ZM)
            } else if (token_length == 5) {
                CHECK_TOKEN_2(POINT, EMPTY)
            } else if (token_length == 7) {
                CHECK_TOKEN_1(POLYGON)
            } else if (token_length == 10) {
                CHECK_TOKEN_2(LINESTRING, MULTIPOINT)
            } else if (token_length == 12) {
                CHECK_TOKEN_1(MULTIPOLYGON)
            } else if (token_length == 15) {
                CHECK_TOKEN_1(MULTILINESTRING)
            } else if (token_length == 18) {
                CHECK_TOKEN_1(GEOMETRYCOLLECTION)
            } else {
                goto error;
            }
#undef CHECK_TOKEN
#undef CHECK_TOKEN_1
#undef CHECK_TOKEN_2

            return;
        } else if (('0' <= c && c <= '9') || c == '-' || c == '+') {
            char *tok_end = NULL;
            tok->token_value = strtod(start, &tok_end);
            if (tok_end == NULL) {
                goto error;
            } else {
                tok->token = WKT_NUMBER;
                tok->start = tok_end;
            }
            return;
        } else if (c == '(' || c == '[') {
            tok->token = WKT_LPAREN;
            tok->start = start + 1;
            return;
        } else if (c == ')' || c == ']') {
            tok->token = WKT_RPAREN;
            tok->start = start + 1;
            return;
        } else if (c == ',') {
            tok->token = WKT_COMMA;
            tok->start = start + 1;
            return;
        } else {
            goto error;
        }
    }
    eof:
    tok->start = tok->end;
    tok->token = WKT_EOF;
    error:
    tok->start = tok->end;
    tok->token = WKT_ERROR;
}

static int wkt_read_point(wkt_tokenizer_t *tok, const geom_header_t *header, const geom_consumer_t *consumer) {
    double coords[header->coord_size];

    for (int i = 0; i < header->coord_size; i++) {
        if (tok->token != WKT_NUMBER) {
            return SQLITE_IOERR;
        }

        coords[i] = tok->token_value;
        wkt_next_token(tok);
    }

    if (consumer->coordinates) {
        consumer->coordinates(consumer, header, 1, coords);
    }

    return SQLITE_OK;
}

#define COORD_BATCH_SIZE 10

static int wkt_read_points(wkt_tokenizer_t *tok, const geom_header_t *header, const geom_consumer_t *consumer) {
    double coords[header->coord_size * COORD_BATCH_SIZE];

    size_t coord_count = 0;
    int coord_offset = 0;

    int more_coords;
    do {
        for (int i = 0; i < header->coord_size; i++) {
            if (tok->token != WKT_NUMBER) {
                return SQLITE_IOERR;
            }

            coords[coord_offset++] = tok->token_value;
            wkt_next_token(tok);
        }
        coord_count++;

        more_coords = tok->token == WKT_COMMA;

        if (coord_count == COORD_BATCH_SIZE || !more_coords) {
            if (consumer->coordinates) {
                consumer->coordinates(consumer, header, coord_count, coords);
            }
            coord_count = 0;
            coord_offset = 0;
        }

        if (more_coords) {
            wkt_next_token(tok);
        }
    } while (more_coords);

    return SQLITE_OK;
}

static int wkt_read_point_text(wkt_tokenizer_t *tok, const geom_header_t *header, const geom_consumer_t *consumer) {
    if (tok->token == WKT_EMPTY) {
        wkt_next_token(tok);
        return SQLITE_OK;
    }

    if (tok->token != WKT_LPAREN) {
        return SQLITE_IOERR;
    } else {
        wkt_next_token(tok);
    }

    int res = wkt_read_point(tok, header, consumer);
    if (res != SQLITE_OK) {
        return res;
    }
    
    if (tok->token != WKT_RPAREN) {
        return SQLITE_IOERR;
    } else {
        wkt_next_token(tok);
        return SQLITE_OK;
    }
}

static int wkt_read_multipoint_text(wkt_tokenizer_t *tok, const geom_header_t *header, const geom_consumer_t *consumer) {
    if (tok->token == WKT_EMPTY) {
        wkt_next_token(tok);
        return SQLITE_OK;
    }

    if (tok->token != WKT_LPAREN) {
        return SQLITE_IOERR;
    } else {
        wkt_next_token(tok);
    }

    geom_header_t point_header;
    point_header.geom_type = GEOM_POINT;
    point_header.coord_type = header->coord_type;
    point_header.coord_size = header->coord_size;

    int more_points;
    do {
        consumer->begin_geometry(consumer, &point_header);
        int res = wkt_read_point_text(tok, &point_header, consumer);
        if (res != SQLITE_OK) {
            return res;
        }
        consumer->end_geometry(consumer, &point_header);

        more_points = tok->token == WKT_COMMA;
        if (more_points) {
            wkt_next_token(tok);
        }
    } while ( more_points );

    if (tok->token != WKT_RPAREN) {
        return SQLITE_IOERR;
    } else {
        wkt_next_token(tok);
        return SQLITE_OK;
    }
}

static int wkt_read_linestring_text(wkt_tokenizer_t *tok, const geom_header_t *header, const geom_consumer_t *consumer) {
    if (tok->token == WKT_EMPTY) {
        wkt_next_token(tok);
        return SQLITE_OK;
    }

    if (tok->token != WKT_LPAREN) {
        return SQLITE_IOERR;
    } else {
        wkt_next_token(tok);
    }

    int res = wkt_read_points(tok, header, consumer);
    if (res != SQLITE_OK) {
        return res;
    }

    if (tok->token != WKT_RPAREN) {
        return SQLITE_IOERR;
    } else {
        wkt_next_token(tok);
        return SQLITE_OK;
    }
}

static int wkt_read_multilinestring_text(wkt_tokenizer_t *tok, const geom_header_t *header, const geom_consumer_t *consumer) {
    if (tok->token == WKT_EMPTY) {
        wkt_next_token(tok);
        return SQLITE_OK;
    }

    if (tok->token != WKT_LPAREN) {
        return SQLITE_IOERR;
    } else {
        wkt_next_token(tok);
    }

    geom_header_t linestring_header;
    linestring_header.geom_type = GEOM_LINESTRING;
    linestring_header.coord_type = header->coord_type;
    linestring_header.coord_size = header->coord_size;

    int more_linestrings;
    do {
        consumer->begin_geometry(consumer, &linestring_header);
        int res = wkt_read_linestring_text(tok, &linestring_header, consumer);
        if (res != SQLITE_OK) {
            return res;
        }
        consumer->end_geometry(consumer, &linestring_header);

        more_linestrings = tok->token == WKT_COMMA;
        if (more_linestrings) {
            wkt_next_token(tok);
        }
    } while (more_linestrings);

    if (tok->token != WKT_RPAREN) {
        return SQLITE_IOERR;
    } else {
        wkt_next_token(tok);
        return SQLITE_OK;
    }
}

static int wkt_read_polygon_text(wkt_tokenizer_t *tok, const geom_header_t *header, const geom_consumer_t *consumer) {
    if (tok->token == WKT_EMPTY) {
        wkt_next_token(tok);
        return SQLITE_OK;
    }

    if (tok->token != WKT_LPAREN) {
        return SQLITE_IOERR;
    } else {
        wkt_next_token(tok);
    }

    geom_header_t ring_header;
    ring_header.geom_type = GEOM_LINEARRING;
    ring_header.coord_type = header->coord_type;
    ring_header.coord_size = header->coord_size;

    int more_rings;
    do {
        consumer->begin_geometry(consumer, &ring_header);
        int res = wkt_read_linestring_text(tok, &ring_header, consumer);
        if (res != SQLITE_OK) {
            return res;
        }
        consumer->end_geometry(consumer, &ring_header);

        more_rings = tok->token == WKT_COMMA;
        if (more_rings) {
            wkt_next_token(tok);
        }
    } while (more_rings);

    if (tok->token != WKT_RPAREN) {
        return SQLITE_IOERR;
    } else {
        wkt_next_token(tok);
        return SQLITE_OK;
    }
}

static int wkt_read_multipolygon_text(wkt_tokenizer_t *tok, const geom_header_t *header, const geom_consumer_t *consumer) {
    if (tok->token == WKT_EMPTY) {
        wkt_next_token(tok);
        return SQLITE_OK;
    }

    if (tok->token != WKT_LPAREN) {
        return SQLITE_IOERR;
    } else {
        wkt_next_token(tok);
    }

    geom_header_t polygon_header;
    polygon_header.geom_type = GEOM_POLYGON;
    polygon_header.coord_type = header->coord_type;
    polygon_header.coord_size = header->coord_size;

    int more_polygons;
    do {
        consumer->begin_geometry(consumer, &polygon_header);
        int res = wkt_read_polygon_text(tok, &polygon_header, consumer);
        if (res != SQLITE_OK) {
            return res;
        }
        consumer->end_geometry(consumer, &polygon_header);

        more_polygons = tok->token == WKT_COMMA;
        if (more_polygons) {
            wkt_next_token(tok);
        }
    } while (more_polygons);

    if (tok->token != WKT_RPAREN) {
        return SQLITE_IOERR;
    } else {
        wkt_next_token(tok);
        return SQLITE_OK;
    }
}

static int wkt_read_geometry_tagged_text(wkt_tokenizer_t *tok, const geom_header_t *parent_header, geom_consumer_t const *consumer, error_t *error);

static int wkt_read_geometrycollection_text(wkt_tokenizer_t *tok, const geom_header_t *header, const geom_consumer_t *consumer) {
    if (tok->token == WKT_EMPTY) {
        wkt_next_token(tok);
        return SQLITE_OK;
    }

    if (tok->token != WKT_LPAREN) {
        return SQLITE_IOERR;
    } else {
        wkt_next_token(tok);
    }

    int more_geometries;
    do {
        int res = wkt_read_geometry_tagged_text(tok, header, consumer, NULL);
        if (res != SQLITE_OK) {
            return res;
        }

        more_geometries = tok->token == WKT_COMMA;
        if (more_geometries) {
            wkt_next_token(tok);
        }
    } while (more_geometries);

    if (tok->token != WKT_RPAREN) {
        return SQLITE_IOERR;
    } else {
        wkt_next_token(tok);
        return SQLITE_OK;
    }
}

static int wkt_read_geometry_tagged_text(wkt_tokenizer_t *tok, const geom_header_t *parent_header, geom_consumer_t const *consumer, error_t *error) {
    int result = SQLITE_OK;
    geom_type_t geometry_type;
    switch (tok->token) {
        case WKT_POINT:
            geometry_type = GEOM_POINT;
            break;
        case WKT_LINESTRING:
            geometry_type = GEOM_LINESTRING;
            break;
        case WKT_POLYGON:
            geometry_type = GEOM_POLYGON;
            break;
        case WKT_MULTIPOINT:
            geometry_type = GEOM_MULTIPOINT;
            break;
        case WKT_MULTILINESTRING:
            geometry_type = GEOM_MULTILINESTRING;
            break;
        case WKT_MULTIPOLYGON:
            geometry_type = GEOM_MULTIPOLYGON;
            break;
        case WKT_GEOMETRYCOLLECTION:
            geometry_type = GEOM_GEOMETRYCOLLECTION;
            break;
        default:
            if (error) error_append(error, "Unsupported WKT geometry type");
            result = SQLITE_IOERR;
            goto exit;
    }

    wkt_next_token(tok);

    coord_type_t coord_type;
    uint32_t coord_size;
    int read_token = 1;
    switch (tok->token) {
        case WKT_Z:
            coord_type = GEOM_XYZ;
            coord_size = 3;
            break;
        case WKT_M:
            coord_type = GEOM_XYM;
            coord_size = 3;
            break;
        case WKT_ZM:
            coord_type = GEOM_XYZM;
            coord_size = 4;
            break;
        default:
            coord_type = GEOM_XY;
            coord_size = 2;
            read_token = 0;
            break;
    }

    if (read_token) {
        wkt_next_token(tok);
    }

    geom_header_t header;
    header.geom_type = geometry_type;
    header.coord_type = coord_type;
    header.coord_size = coord_size;

    if (parent_header != NULL && parent_header->coord_type != header.coord_type) {
        result = SQLITE_IOERR;
        goto exit;
    }
    
    result = consumer->begin_geometry(consumer, &header);
    if (result != SQLITE_OK) {
        goto exit;
    }
    
    switch (geometry_type) {
        case GEOM_POINT:
            result = wkt_read_point_text(tok, &header, consumer);
            break;
        case GEOM_LINESTRING:
            result = wkt_read_linestring_text(tok, &header, consumer);
            break;
        case GEOM_POLYGON:
            result = wkt_read_polygon_text(tok, &header, consumer);
            break;
        case GEOM_MULTIPOINT:
            result = wkt_read_multipoint_text(tok, &header, consumer);
            break;
        case GEOM_MULTILINESTRING:
            result = wkt_read_multilinestring_text(tok, &header, consumer);
            break;
        case GEOM_MULTIPOLYGON:
            result = wkt_read_multipolygon_text(tok, &header, consumer);
            break;
        case GEOM_GEOMETRYCOLLECTION:
            result = wkt_read_geometrycollection_text(tok, &header, consumer);
            break;
        default:
            result = SQLITE_IOERR;
    }
    if (result != SQLITE_OK) {
        goto exit;
    }

    result = consumer->end_geometry(consumer, &header);

  exit:
    return result;
}

int wkt_read_geometry(char const *data, size_t length, geom_consumer_t const *consumer, error_t *error) {
    int result = SQLITE_OK;
    
    result = consumer->begin(consumer);
    if (result != SQLITE_OK) {
        goto exit;
    }

    wkt_tokenizer_t tok;
    tok.start = data;
    tok.end = data + length;
    wkt_next_token(&tok);
    
    result = wkt_read_geometry_tagged_text(&tok, NULL, consumer, error);
    if (result != SQLITE_OK) {
        goto exit;
    }

    result = consumer->end(consumer);

  exit:
    return result;
}
