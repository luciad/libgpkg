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
#include "sqlite.h"
#include "wkt.h"


static int wkt_begin_geometry(const geom_consumer_t *consumer, const geom_header_t *header, errorstream_t *error) {
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

  if (writer->offset == 0 ||
      writer->type[writer->offset - 1] == GEOM_GEOMETRYCOLLECTION ||
      writer->type[writer->offset - 1] == GEOM_COMPOUNDCURVE ||
      writer->type[writer->offset - 1] == GEOM_CURVEPOLYGON) {
    switch (header->geom_type) {
      case GEOM_POINT:
        result = strbuf_append(&writer->strbuf, "Point ");
        break;
      case GEOM_LINESTRING:

        if (writer->offset == 0 ||
            (writer->type[writer->offset - 1] != GEOM_COMPOUNDCURVE &&
             writer->type[writer->offset - 1] != GEOM_CURVEPOLYGON)) {
          result = strbuf_append(&writer->strbuf, "LineString ");
        }
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
      case GEOM_CIRCULARSTRING:
        result = strbuf_append(&writer->strbuf, "CircularString ");
        break;
      case GEOM_COMPOUNDCURVE:
        result = strbuf_append(&writer->strbuf, "CompoundCurve ");
        break;
      case GEOM_CURVEPOLYGON:
        result = strbuf_append(&writer->strbuf, "CurvePolygon ");
        break;
      default:
        result = SQLITE_ERROR;
        break;
    }

    if (result != SQLITE_OK) {
      goto exit;
    }

    if (header->geom_type != GEOM_LINESTRING || (writer->offset == 0 ||
        (writer->type[writer->offset - 1] != GEOM_COMPOUNDCURVE &&
         writer->type[writer->offset - 1] != GEOM_CURVEPOLYGON))) {
      if (header->coord_type == GEOM_XYZ) {
        result = strbuf_append(&writer->strbuf, "Z ");
      } else if (header->coord_type == GEOM_XYM) {
        result = strbuf_append(&writer->strbuf, "M ");
      } else if (header->coord_type == GEOM_XYZM) {
        result = strbuf_append(&writer->strbuf, "ZM ");
      }
    }

    if (result != SQLITE_OK) {
      goto exit;
    }
  }

exit:
  return result;
}

#define WKT_COORD "%.10g"
#define WKT_COORD_2 WKT_COORD " " WKT_COORD
#define WKT_COORD_3 WKT_COORD_2 " " WKT_COORD
#define WKT_COORD_4 WKT_COORD_3 " " WKT_COORD

static int wkt_coordinates(const geom_consumer_t *consumer, const geom_header_t *header, size_t point_count, const double *coords, int skip_coords, errorstream_t *error) {
  int result = SQLITE_OK;

  wkt_writer_t *writer = (wkt_writer_t *) consumer;

  int first = writer->children[writer->offset] == 0;
  if (first) {
    result = strbuf_append(&writer->strbuf, "(");
  }
  writer->children[writer->offset]++;

  if (result != SQLITE_OK) {
    goto exit;
  }

  int offset = skip_coords;
  point_count = (offset == 0) ? point_count : (point_count - (offset / header->coord_size));
  if (header->coord_size == 2) {
    for (size_t i = 0; i < point_count; i++) {
      double x = coords[offset++];
      double y = coords[offset++];

      if (first) {
        result = strbuf_append(&writer->strbuf, WKT_COORD_2, x, y);
        first = 0;
      } else {
        result = strbuf_append(&writer->strbuf, ", "WKT_COORD_2, x, y);
      }

      if (result != SQLITE_OK) {
        goto exit;
      }
    }
  } else if (header->coord_size == 3) {
    for (size_t i = 0; i < point_count; i++) {
      double x = coords[offset++];
      double y = coords[offset++];
      double zm = coords[offset++];
      if (first) {
        result = strbuf_append(&writer->strbuf, WKT_COORD_3, x, y, zm);
        first = 0;
      } else {
        result = strbuf_append(&writer->strbuf, ", "WKT_COORD_3, x, y, zm);
      }

      if (result != SQLITE_OK) {
        goto exit;
      }
    }
  } else if (header->coord_size == 4) {
    for (size_t i = 0; i < point_count; i++) {
      double x = coords[offset++];
      double y = coords[offset++];
      double z = coords[offset++];
      double m = coords[offset++];
      if (first) {
        result = strbuf_append(&writer->strbuf, WKT_COORD_4, x, y, z, m);
        first = 0;
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

static int wkt_end_geometry(const geom_consumer_t *consumer, const geom_header_t *header, errorstream_t *error) {
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

geom_consumer_t *wkt_writer_geom_consumer(wkt_writer_t *writer) {
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

typedef enum {
  WKT_POINT,
  WKT_POLYGON,
  WKT_LINESTRING,
  WKT_MULTIPOINT,
  WKT_CURVEPOLYGON,
  WKT_MULTIPOLYGON,
  WKT_COMPOUNDCURVE,
  WKT_MULTILINESTRING,
  WKT_GEOMETRYCOLLECTION,
  WKT_CIRCULARSTRING,
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

typedef struct {
  const char *start;
  const char *end;
  const char *position;

  const char *token_start;
  int token_position;
  int token_length;
  wkt_token token;
  double token_value;
  i18n_locale_t *locale;
} wkt_tokenizer_t;

typedef int(*read_body_function)(wkt_tokenizer_t *, const geom_header_t *, geom_consumer_t const *, errorstream_t *);
static int get_read_body_function(wkt_tokenizer_t *tok, wkt_token geom_token, read_body_function *read_body, geom_type_t *geometry_type, errorstream_t *error);

static void wkt_tokenizer_init(wkt_tokenizer_t *tok, const char *data, size_t length, i18n_locale_t *locale) {
  tok->start = data;
  tok->position = data;
  tok->token_position = 0;
  tok->end = data + length;
  tok->locale = locale;
}

static void wkt_tokenizer_error(wkt_tokenizer_t *tok, errorstream_t *error, const char *msg) {
  if (tok-> token_length > 0) {
    error_append(error, "%s at column %d: %.*s", msg, tok->token_position, tok->token_length, tok->token_start);
  } else {
    error_append(error, "%s at column %d", msg, tok->token_position);
  }
}

static void wkt_tokenizer_next(wkt_tokenizer_t *tok) {
  const char *start = tok->position;
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

    tok->token_start = start;
    tok->token_position = (start - tok->start);
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
      tok->position = tok_end;
      tok->token_length = token_length;

#define CHECK_TOKEN(token_name) if ( sqlite3_strnicmp( #token_name, start, token_length ) == 0) { tok->token = WKT_##token_name; }
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
        CHECK_TOKEN_2(MULTIPOLYGON, CURVEPOLYGON)
      } else if (token_length == 13) {
        CHECK_TOKEN_1(COMPOUNDCURVE)
      } else if (token_length == 14) {
        CHECK_TOKEN_1(CIRCULARSTRING)
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
      tok->token_value = i18n_strtod(start, &tok_end, tok->locale);
      if (tok_end == NULL) {
        tok->token_length = 0;
        goto error;
      } else {
        tok->token = WKT_NUMBER;
        tok->position = tok_end;
        tok->token_length = tok_end - start;
      }
      return;
    } else if (c == '(' || c == '[') {
      tok->token = WKT_LPAREN;
      tok->position = start + 1;
      tok->token_length = 1;
      return;
    } else if (c == ')' || c == ']') {
      tok->token = WKT_RPAREN;
      tok->position = start + 1;
      tok->token_length = 1;
      return;
    } else if (c == ',') {
      tok->token = WKT_COMMA;
      tok->position = start + 1;
      tok->token_length = 1;
      return;
    } else {
      tok->token_length = 0;
      goto error;
    }
  }
eof:
  tok->position = tok->end;
  tok->token = WKT_EOF;
  tok->token_length = 0;
  return;
error:
  tok->position = tok->end;
  tok->token = WKT_ERROR;
  return;
}

static int wkt_read_dimension_info(wkt_tokenizer_t *tok, const geom_header_t *parent_header, geom_header_t *header, errorstream_t *error);

static int wkt_read_point(wkt_tokenizer_t *tok, const geom_header_t *header, const geom_consumer_t *consumer, errorstream_t *error) {
  int result = SQLITE_OK;
  double coords[GEOM_MAX_COORD_SIZE];

  for (uint32_t i = 0; i < header->coord_size; i++) {
    if (tok->token != WKT_NUMBER) {
      if (error) {
        wkt_tokenizer_error(tok, error, "Expected number");
      }
      result = SQLITE_IOERR;
      goto exit;
    }

    coords[i] = tok->token_value;
    wkt_tokenizer_next(tok);
  }

  if (consumer->coordinates) {
    result = consumer->coordinates(consumer, header, 1, coords, 0, error);
  }

exit:
  return result;
}

#define COORD_BATCH_SIZE 10

static int wkt_read_points(wkt_tokenizer_t *tok, const geom_header_t *header, const geom_consumer_t *consumer, errorstream_t *error) {
  int result = SQLITE_OK;
  double coords[GEOM_MAX_COORD_SIZE * COORD_BATCH_SIZE];

  size_t coord_count = 0;
  int max_coords_to_read = COORD_BATCH_SIZE;
  int coord_offset = 0;
  int skip_coords = 0;

  if (header->geom_type == GEOM_CIRCULARSTRING) {
    max_coords_to_read = COORD_BATCH_SIZE - ((COORD_BATCH_SIZE - 3) % 2);
  }


  int more_coords;
  do {
    for (uint32_t i = 0; i < header->coord_size; i++) {
      if (tok->token != WKT_NUMBER) {
        if (error) {
          wkt_tokenizer_error(tok, error, "Expected number");
        }
        result = SQLITE_IOERR;
        goto exit;
      }

      coords[coord_offset++] = tok->token_value;
      wkt_tokenizer_next(tok);
    }
    coord_count++;

    more_coords = tok->token == WKT_COMMA;

    if (coord_count == max_coords_to_read || !more_coords) {
      if (header->geom_type == GEOM_CIRCULARSTRING) {
        if ((coord_count - 3) % 2 != 0 && coord_count != 0) {
          if (error) {
            error_append(error, "Error CircularString requires 3+2n points or has to be EMPTY");
          }
          result = SQLITE_IOERR;
          goto exit;
        }
      }

      if (consumer->coordinates) {
        result = consumer->coordinates(consumer, header, coord_count, coords, skip_coords, error);
        if (result != SQLITE_OK) {
          goto exit;
        }
      }
      if (header->geom_type == GEOM_CIRCULARSTRING && more_coords) {
        for (uint32_t i = 0; i < header->coord_size; i++) {
          coords[i] = coords[((coord_count - 1) * header->coord_size) + i];
        }
        coord_offset = header->coord_size;
        skip_coords = header->coord_size;
        coord_count = 1;
      } else {
        coord_offset = 0;
        coord_count = 0;
      }
    }

    if (more_coords) {
      wkt_tokenizer_next(tok);
    }
  } while (more_coords);

exit:
  return result;
}

static int wkt_read_point_text(wkt_tokenizer_t *tok, const geom_header_t *header, const geom_consumer_t *consumer, errorstream_t *error) {
  if (tok->token == WKT_EMPTY) {
    wkt_tokenizer_next(tok);
    return SQLITE_OK;
  }

  if (tok->token != WKT_LPAREN) {
    if (error) {
      wkt_tokenizer_error(tok, error, "Expected '(' or 'empty'");
    }
    return SQLITE_IOERR;
  } else {
    wkt_tokenizer_next(tok);
  }

  int res = wkt_read_point(tok, header, consumer, error);
  if (res != SQLITE_OK) {
    return res;
  }

  if (tok->token != WKT_RPAREN) {
    if (error) {
      wkt_tokenizer_error(tok, error, "Expected ')'");
    }
    return SQLITE_IOERR;
  } else {
    wkt_tokenizer_next(tok);
    return SQLITE_OK;
  }
}

static int wkt_read_multipoint_text(wkt_tokenizer_t *tok, const geom_header_t *header, const geom_consumer_t *consumer, errorstream_t *error) {
  if (tok->token == WKT_EMPTY) {
    wkt_tokenizer_next(tok);
    return SQLITE_OK;
  }

  if (tok->token != WKT_LPAREN) {
    if (error) {
      wkt_tokenizer_error(tok, error, "Expected '(' or 'empty'");
    }
    return SQLITE_IOERR;
  } else {
    wkt_tokenizer_next(tok);
  }

  geom_header_t point_header;
  point_header.geom_type = GEOM_POINT;
  point_header.coord_type = header->coord_type;
  point_header.coord_size = header->coord_size;

  int more_points;
  do {
    int res = consumer->begin_geometry(consumer, &point_header, error);
    if (res != SQLITE_OK) {
      return res;
    }

    res = wkt_read_point_text(tok, &point_header, consumer, error);
    if (res != SQLITE_OK) {
      return res;
    }

    res = consumer->end_geometry(consumer, &point_header, error);
    if (res != SQLITE_OK) {
      return res;
    }

    more_points = tok->token == WKT_COMMA;
    if (more_points) {
      wkt_tokenizer_next(tok);
    }
  } while (more_points);

  if (tok->token != WKT_RPAREN) {
    if (error) {
      wkt_tokenizer_error(tok, error, "Expected ')'");
    }
    return SQLITE_IOERR;
  } else {
    wkt_tokenizer_next(tok);
    return SQLITE_OK;
  }
}

static int wkt_read_linestring_text(wkt_tokenizer_t *tok, const geom_header_t *header, const geom_consumer_t *consumer, errorstream_t *error) {
  if (tok->token == WKT_EMPTY) {
    wkt_tokenizer_next(tok);
    return SQLITE_OK;
  }

  if (tok->token != WKT_LPAREN) {
    if (error) {
      wkt_tokenizer_error(tok, error, "Expected '(' or 'empty'");
    }
    return SQLITE_IOERR;
  } else {
    wkt_tokenizer_next(tok);
  }

  int res = wkt_read_points(tok, header, consumer, error);
  if (res != SQLITE_OK) {
    return res;
  }

  if (tok->token != WKT_RPAREN) {
    if (error) {
      wkt_tokenizer_error(tok, error, "Expected ')'");
    }
    return SQLITE_IOERR;
  } else {
    wkt_tokenizer_next(tok);
    return SQLITE_OK;
  }
}

static int wkt_read_multilinestring_text(wkt_tokenizer_t *tok, const geom_header_t *header, const geom_consumer_t *consumer, errorstream_t *error) {
  if (tok->token == WKT_EMPTY) {
    wkt_tokenizer_next(tok);
    return SQLITE_OK;
  }

  if (tok->token != WKT_LPAREN) {
    if (error) {
      wkt_tokenizer_error(tok, error, "Expected '(' or 'empty'");
    }
    return SQLITE_IOERR;
  } else {
    wkt_tokenizer_next(tok);
  }

  geom_header_t linestring_header;
  linestring_header.geom_type = GEOM_LINESTRING;
  linestring_header.coord_type = header->coord_type;
  linestring_header.coord_size = header->coord_size;

  int more_linestrings;
  do {
    int res = consumer->begin_geometry(consumer, &linestring_header, error);
    if (res != SQLITE_OK) {
      return res;
    }

    res = wkt_read_linestring_text(tok, &linestring_header, consumer, error);
    if (res != SQLITE_OK) {
      return res;
    }
    res = consumer->end_geometry(consumer, &linestring_header, error);
    if (res != SQLITE_OK) {
      return res;
    }

    more_linestrings = tok->token == WKT_COMMA;
    if (more_linestrings) {
      wkt_tokenizer_next(tok);
    }
  } while (more_linestrings);

  if (tok->token != WKT_RPAREN) {
    if (error) {
      wkt_tokenizer_error(tok, error, "Expected ')'");
    }
    return SQLITE_IOERR;
  } else {
    wkt_tokenizer_next(tok);
    return SQLITE_OK;
  }
}

static int wkt_read_polygon_text(wkt_tokenizer_t *tok, const geom_header_t *header, const geom_consumer_t *consumer, errorstream_t *error) {
  if (tok->token == WKT_EMPTY) {
    wkt_tokenizer_next(tok);
    return SQLITE_OK;
  }

  if (tok->token != WKT_LPAREN) {
    if (error) {
      wkt_tokenizer_error(tok, error, "Expected '(' or 'empty'");
    }
    return SQLITE_IOERR;
  } else {
    wkt_tokenizer_next(tok);
  }

  geom_header_t ring_header;
  ring_header.geom_type = GEOM_LINEARRING;
  ring_header.coord_type = header->coord_type;
  ring_header.coord_size = header->coord_size;

  int more_rings;
  do {
    int res = consumer->begin_geometry(consumer, &ring_header, error);
    if (res != SQLITE_OK) {
      return res;
    }

    res = wkt_read_linestring_text(tok, &ring_header, consumer, error);
    if (res != SQLITE_OK) {
      return res;
    }
    res = consumer->end_geometry(consumer, &ring_header, error);
    if (res != SQLITE_OK) {
      return res;
    }

    more_rings = tok->token == WKT_COMMA;
    if (more_rings) {
      wkt_tokenizer_next(tok);
    }
  } while (more_rings);

  if (tok->token != WKT_RPAREN) {
    if (error) {
      wkt_tokenizer_error(tok, error, "Expected ')'");
    }
    return SQLITE_IOERR;
  } else {
    wkt_tokenizer_next(tok);
    return SQLITE_OK;
  }
}

static int wkt_read_multipolygon_text(wkt_tokenizer_t *tok, const geom_header_t *header, const geom_consumer_t *consumer, errorstream_t *error) {
  if (tok->token == WKT_EMPTY) {
    wkt_tokenizer_next(tok);
    return SQLITE_OK;
  }

  if (tok->token != WKT_LPAREN) {
    if (error) {
      wkt_tokenizer_error(tok, error, "Expected '(' or 'empty'");
    }
    return SQLITE_IOERR;
  } else {
    wkt_tokenizer_next(tok);
  }

  geom_header_t polygon_header;
  polygon_header.geom_type = GEOM_POLYGON;
  polygon_header.coord_type = header->coord_type;
  polygon_header.coord_size = header->coord_size;

  int more_polygons;
  do {
    int res = consumer->begin_geometry(consumer, &polygon_header, error);
    if (res != SQLITE_OK) {
      return res;
    }

    res = wkt_read_polygon_text(tok, &polygon_header, consumer, error);
    if (res != SQLITE_OK) {
      return res;
    }
    res = consumer->end_geometry(consumer, &polygon_header, error);
    if (res != SQLITE_OK) {
      return res;
    }

    more_polygons = tok->token == WKT_COMMA;
    if (more_polygons) {
      wkt_tokenizer_next(tok);
    }
  } while (more_polygons);

  if (tok->token != WKT_RPAREN) {
    if (error) {
      wkt_tokenizer_error(tok, error, "Expected ')'");
    }
    return SQLITE_IOERR;
  } else {
    wkt_tokenizer_next(tok);
    return SQLITE_OK;
  }
}

static int wkt_read_curvepolygon_text(wkt_tokenizer_t *tok, const geom_header_t *header, const geom_consumer_t *consumer, errorstream_t *error) {
  if (tok->token == WKT_EMPTY) {
    wkt_tokenizer_next(tok);
    return SQLITE_OK;
  }

  if (tok->token != WKT_LPAREN) {
    if (error) {
      wkt_tokenizer_error(tok, error, "Expected '(' or 'empty'");
    }
    return SQLITE_IOERR;
  } else {
    wkt_tokenizer_next(tok);
  }

  int more_curves;
  do {
    int res;
    wkt_token geom_type = tok->token;

    geom_header_t child_header;
    if (geom_type == WKT_LPAREN) {
      geom_type = WKT_LINESTRING;
      child_header.geom_type = GEOM_LINESTRING;
      child_header.coord_type = header->coord_type;
      child_header.coord_size = header->coord_size;
    } else if (geom_type == WKT_LINESTRING) {
      if (error) {
        wkt_tokenizer_error(tok, error, "LineString keyword not allowed in curvepolygon");
      }
      return SQLITE_IOERR;
    } else {
      wkt_read_dimension_info(tok, header, &child_header, error);
      if (geom_type == WKT_CIRCULARSTRING) {
        child_header.geom_type = GEOM_CIRCULARSTRING;
      } else if (geom_type == WKT_COMPOUNDCURVE) {
        child_header.geom_type = GEOM_COMPOUNDCURVE;
      }
    }

    if (geom_type == WKT_LINESTRING || geom_type == WKT_CIRCULARSTRING || geom_type == WKT_COMPOUNDCURVE) {
      res = consumer->begin_geometry(consumer, &child_header, error);
    } else {
      if (error) {
        wkt_tokenizer_error(tok, error, "CurvePolygon can only contain LineString, CircularString or CompoundCurve");
      }
      return SQLITE_IOERR;
    }

    if (res != SQLITE_OK) {
      return res;
    }

    read_body_function read_body = NULL;
    geom_type_t geometry_type = GEOM_GEOMETRY;
    res = get_read_body_function(tok, geom_type, &read_body, &geometry_type, error);

    if (res != SQLITE_OK) {
      return res;
    }

    res = read_body(tok, &child_header, consumer, error);

    if (res != SQLITE_OK) {
      return res;
    }

    res = consumer->end_geometry(consumer, &child_header, error);


    if (res != SQLITE_OK) {
      return res;
    }

    more_curves = tok->token == WKT_COMMA;
    if (more_curves) {
      wkt_tokenizer_next(tok);
    }

  } while (more_curves);

  if (tok->token != WKT_RPAREN) {
    if (error) {
      wkt_tokenizer_error(tok, error, "Expected ')'");
    }
    return SQLITE_IOERR;
  } else {
    wkt_tokenizer_next(tok);
    return SQLITE_OK;
  }
  return SQLITE_OK;
}

static int wkt_read_geometry_tagged_text(wkt_tokenizer_t *tok, const geom_header_t *parent_header, geom_consumer_t const *consumer, errorstream_t *error);

static int wkt_read_circularstring_text(wkt_tokenizer_t *tok, const geom_header_t *header, const geom_consumer_t *consumer, errorstream_t *error) {
  return wkt_read_linestring_text(tok, header, consumer, error);
}

static int wkt_read_compoundcurve_text(wkt_tokenizer_t *tok, const geom_header_t *header, const geom_consumer_t *consumer, errorstream_t *error) {
  if (tok->token == WKT_EMPTY) {
    wkt_tokenizer_next(tok);
    return SQLITE_OK;
  }

  if (tok->token != WKT_LPAREN) {
    if (error) {
      wkt_tokenizer_error(tok, error, "Expected '(' or 'empty'");
    }
    return SQLITE_IOERR;
  } else {
    wkt_tokenizer_next(tok);
  }

  int more_curves;
  do {
    int res;
    wkt_token geom_type = tok->token;

    geom_header_t child_header;

    if (geom_type == WKT_LPAREN) {
      geom_type = WKT_LINESTRING;
      child_header.geom_type = GEOM_LINESTRING;
      child_header.coord_type = header->coord_type;
      child_header.coord_size = header->coord_size;
    } else if (geom_type == WKT_LINESTRING) {
      if (error) {
        wkt_tokenizer_error(tok, error, "LineString keyword not allowed in compoundcurve");
      }
      return SQLITE_IOERR;
    } else {
      wkt_read_dimension_info(tok, header, &child_header, error);
      child_header.geom_type = GEOM_CIRCULARSTRING;
    }

    if (geom_type == WKT_LINESTRING || geom_type == WKT_CIRCULARSTRING) {
      res = consumer->begin_geometry(consumer, &child_header, error);
    } else {
      if (error) {
        wkt_tokenizer_error(tok, error, "CompoundCurve can only contain LineString or CircularString");
      }
      return SQLITE_IOERR;
    }

    if (res != SQLITE_OK) {
      return res;
    }

    read_body_function read_body = NULL;
    geom_type_t geometry_type = GEOM_GEOMETRY;
    res = get_read_body_function(tok, geom_type, &read_body, &geometry_type, error);

    if (res != SQLITE_OK) {
      return res;
    }

    res = read_body(tok, &child_header, consumer, error);


    if (geom_type == WKT_LINESTRING) {
      res = consumer->end_geometry(consumer, &child_header, error);
    } else if (geom_type == WKT_CIRCULARSTRING) {
      res = consumer->end_geometry(consumer, &child_header, error);
    }

    if (res != SQLITE_OK) {
      return res;
    }

    more_curves = tok->token == WKT_COMMA;
    if (more_curves) {
      wkt_tokenizer_next(tok);
    }

  } while (more_curves);

  if (tok->token != WKT_RPAREN) {
    if (error) {
      wkt_tokenizer_error(tok, error, "Expected ')'");
    }
    return SQLITE_IOERR;
  } else {
    wkt_tokenizer_next(tok);
    return SQLITE_OK;
  }
}

static int wkt_read_geometrycollection_text(wkt_tokenizer_t *tok, const geom_header_t *header, const geom_consumer_t *consumer, errorstream_t *error) {
  if (tok->token == WKT_EMPTY) {
    wkt_tokenizer_next(tok);
    return SQLITE_OK;
  }

  if (tok->token != WKT_LPAREN) {
    if (error) {
      wkt_tokenizer_error(tok, error, "Expected '(' or 'empty'");
    }
    return SQLITE_IOERR;
  } else {
    wkt_tokenizer_next(tok);
  }

  int more_geometries;
  do {
    int res = wkt_read_geometry_tagged_text(tok, header, consumer, error);
    if (res != SQLITE_OK) {
      return res;
    }

    more_geometries = tok->token == WKT_COMMA;
    if (more_geometries) {
      wkt_tokenizer_next(tok);
    }
  } while (more_geometries);

  if (tok->token != WKT_RPAREN) {
    if (error) {
      wkt_tokenizer_error(tok, error, "Expected ')'");
    }
    return SQLITE_IOERR;
  } else {
    wkt_tokenizer_next(tok);
    return SQLITE_OK;
  }
}

static int wkt_read_dimension_info(wkt_tokenizer_t *tok, const geom_header_t *parent_header, geom_header_t *header, errorstream_t *error) {
  wkt_tokenizer_next(tok);

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
    case WKT_EMPTY:
    case WKT_LPAREN:
      coord_type = GEOM_XY;
      coord_size = 2;
      read_token = 0;
      break;
    default:
      if (error) {
        wkt_tokenizer_error(tok, error, "Unexpected token");
      }
      return SQLITE_IOERR;
  }

  if (read_token) {
    wkt_tokenizer_next(tok);
  }

  header->coord_type = coord_type;
  header->coord_size = coord_size;

  if (parent_header != NULL && parent_header->coord_type != header->coord_type) {
    if (error) {
      wkt_tokenizer_error(tok, error, "Child dimension differs from parent dimension");
    }
    return SQLITE_IOERR;
  }

  return SQLITE_OK;
}

static int get_read_body_function(wkt_tokenizer_t *tok, wkt_token geom_token, read_body_function *read_body, geom_type_t *geometry_type, errorstream_t *error) {

  switch (geom_token) {
    case WKT_POINT:
      *geometry_type = GEOM_POINT;
      *read_body = wkt_read_point_text;
      break;
    case WKT_LINESTRING:
      *geometry_type = GEOM_LINESTRING;
      *read_body = wkt_read_linestring_text;
      break;
    case WKT_POLYGON:
      *geometry_type = GEOM_POLYGON;
      *read_body = wkt_read_polygon_text;
      break;
    case WKT_MULTIPOINT:
      *geometry_type = GEOM_MULTIPOINT;
      *read_body = wkt_read_multipoint_text;
      break;
    case WKT_MULTILINESTRING:
      *geometry_type = GEOM_MULTILINESTRING;
      *read_body = wkt_read_multilinestring_text;
      break;
    case WKT_MULTIPOLYGON:
      *geometry_type = GEOM_MULTIPOLYGON;
      *read_body = wkt_read_multipolygon_text;
      break;
    case WKT_GEOMETRYCOLLECTION:
      *geometry_type = GEOM_GEOMETRYCOLLECTION;
      *read_body = wkt_read_geometrycollection_text;
      break;
    case WKT_CURVEPOLYGON:
      *geometry_type = GEOM_CURVEPOLYGON;
      *read_body = wkt_read_curvepolygon_text;
      break;
    case WKT_COMPOUNDCURVE:
      *geometry_type = GEOM_COMPOUNDCURVE;
      *read_body = wkt_read_compoundcurve_text;
      break;
    case WKT_CIRCULARSTRING:
      *geometry_type = GEOM_CIRCULARSTRING;
      *read_body = wkt_read_circularstring_text;
      break;
    default:
      if (error) {
        wkt_tokenizer_error(tok, error, "Unsupported WKT geometry type");
      }
      return SQLITE_IOERR;
  }
  return SQLITE_OK;
}


static int wkt_read_geometry_tagged_text(wkt_tokenizer_t *tok, const geom_header_t *parent_header, geom_consumer_t const *consumer, errorstream_t *error) {
  int result = SQLITE_OK;
  geom_type_t geometry_type = GEOM_GEOMETRY;
  read_body_function read_body = NULL;
  result = get_read_body_function(tok, tok->token, &read_body, &geometry_type, error);

  if (result != SQLITE_OK) {
    goto exit;
  }

  geom_header_t header;
  header.geom_type = geometry_type;
  result = wkt_read_dimension_info(tok, parent_header, &header, error);
  if (result != SQLITE_OK) {
    goto exit;
  }

  result = consumer->begin_geometry(consumer, &header, error);
  if (result != SQLITE_OK) {
    goto exit;
  }

  result = read_body(tok, &header, consumer, error);
  if (result != SQLITE_OK) {
    goto exit;
  }

  result = consumer->end_geometry(consumer, &header, error);

exit:
  return result;
}

int wkt_read_geometry(char const *data, size_t length, geom_consumer_t const *consumer, i18n_locale_t *locale, errorstream_t *error) {
  int result = SQLITE_OK;

  result = consumer->begin(consumer, error);
  if (result != SQLITE_OK) {
    goto exit;
  }

  wkt_tokenizer_t tok;
  wkt_tokenizer_init(&tok, data, length, locale);
  wkt_tokenizer_next(&tok);

  result = wkt_read_geometry_tagged_text(&tok, NULL, consumer, error);
  if (result != SQLITE_OK) {
    goto exit;
  }

  result = consumer->end(consumer, error);

exit:
  return result;
}
