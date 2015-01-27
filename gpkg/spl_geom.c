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
#include "fp.h"
#include "spl_geom.h"
#include "sqlite.h"
#include "blobio.h"
#include "geomio.h"

#define SPB_BIG_ENDIAN 0x00
#define SPB_LITTLE_ENDIAN 0x01

#define CHECK_ENV_COMP(spb, comp, error) \
    if (spb->envelope.has_env_##comp) { \
      if ((spb->empty && (!fp_isnan(spb->envelope.min_##comp) || !fp_isnan(spb->envelope.max_##comp))) || spb->envelope.min_##comp > spb->envelope.max_##comp) {\
          if (error) error_append(error, "SPB envelope min" #comp " > max" #comp ": [min: %g, max: %g]", spb->envelope.min_##comp, spb->envelope.max_##comp);\
          return SQLITE_IOERR;\
      }\
    }
#define CHECK_ENV(spb, error) CHECK_ENV_COMP(spb, x, error) CHECK_ENV_COMP(spb, y, error) CHECK_ENV_COMP(spb, z, error) CHECK_ENV_COMP(spb, m, error)

int spb_read_header(binstream_t *stream, geom_blob_header_t *spb, errorstream_t *error) {
  uint8_t start;
  if (binstream_read_u8(stream, &start) != SQLITE_OK) {
    return SQLITE_IOERR;
  }

  if (start != 0x00) {
    if (error) {
      error_append(error, "Incorrect SPB START value [expected: 00, actual:%x]", start);
    }
    return SQLITE_IOERR;
  }

  uint8_t endian;
  if (binstream_read_u8(stream, &endian)) {
    return SQLITE_IOERR;
  }

  if (endian != SPB_BIG_ENDIAN && endian != SPB_LITTLE_ENDIAN) {
    if (error) {
      error_append(error, "Incorrect SPB ENDIAN value [expected: 00 or 01, actual:%x]", endian);
    }
    return SQLITE_IOERR;
  }

  binstream_set_endianness(stream, endian == SPB_BIG_ENDIAN ? BIG : LITTLE);
  if (binstream_read_i32(stream, &spb->srid) != SQLITE_OK) {
    return SQLITE_IOERR;
  }

  spb->envelope.has_env_x = 1;
  spb->envelope.has_env_y = 1;
  spb->envelope.has_env_z = 0;
  spb->envelope.has_env_m = 0;
  if (binstream_read_double(stream, &spb->envelope.min_x)) {
    return SQLITE_IOERR;
  }
  if (binstream_read_double(stream, &spb->envelope.min_y)) {
    return SQLITE_IOERR;
  }
  if (binstream_read_double(stream, &spb->envelope.max_x)) {
    return SQLITE_IOERR;
  }
  if (binstream_read_double(stream, &spb->envelope.max_y)) {
    return SQLITE_IOERR;
  }

  spb->empty = fp_isnan(spb->envelope.min_x) && fp_isnan(spb->envelope.max_x) && fp_isnan(spb->envelope.min_y) && fp_isnan(spb->envelope.max_y);

  CHECK_ENV(spb, error)

  return SQLITE_OK;
}

int spb_write_header(binstream_t *stream, geom_blob_header_t *spb, errorstream_t *error) {
  CHECK_ENV(spb, error)

  if (binstream_write_u8(stream, 0x00)) {
    return SQLITE_IOERR;
  }

  uint8_t endian = binstream_get_endianness(stream) == LITTLE ? SPB_LITTLE_ENDIAN : SPB_BIG_ENDIAN;
  if (binstream_write_u8(stream, endian)) {
    return SQLITE_IOERR;
  }

  if (binstream_write_i32(stream, spb->srid)) {
    return SQLITE_IOERR;
  }

  if (binstream_write_double(stream, spb->envelope.min_x)) {
    return SQLITE_IOERR;
  }
  if (binstream_write_double(stream, spb->envelope.min_y)) {
    return SQLITE_IOERR;
  }
  if (binstream_write_double(stream, spb->envelope.max_x)) {
    return SQLITE_IOERR;
  }
  if (binstream_write_double(stream, spb->envelope.max_y)) {
    return SQLITE_IOERR;
  }

  return SQLITE_OK;
}

static int spb_begin_geometry(const geom_consumer_t *consumer, const geom_header_t *header, errorstream_t *error) {
  int result = SQLITE_OK;

  geom_blob_writer_t *writer = (geom_blob_writer_t *) consumer;
  wkb_writer_t *wkb = &writer->wkb_writer;

  if (wkb->offset < 0) {
    writer->geom_type = header->geom_type;

    result = binstream_relseek(&wkb->stream, 38);
    if (result != SQLITE_OK) {
      goto exit;
    }
  }

  geom_consumer_t *wkb_consumer = wkb_writer_geom_consumer(wkb);
  result = wkb_consumer->begin_geometry(wkb_consumer, header, error);
exit:
  return result;
}

static int spb_coordinates(const geom_consumer_t *consumer, const geom_header_t *header, size_t point_count, const double *coords, int skip_coords, errorstream_t *error) {
  int result = SQLITE_OK;

  if (point_count <= 0) {
    goto exit;
  }

  geom_blob_writer_t *writer = (geom_blob_writer_t *) consumer;
  wkb_writer_t *wkb = &writer->wkb_writer;
  geom_consumer_t *wkb_consumer = wkb_writer_geom_consumer(wkb);
  result = wkb_consumer->coordinates(wkb_consumer, header, point_count, coords, skip_coords, error);
  if (result != SQLITE_OK) {
    goto exit;
  }

  if (header->geom_type == GEOM_POINT) {
    int allnan = 1;
    for (uint32_t i = 0; i < header->coord_size; i++) {
      allnan &= fp_isnan(coords[i]);
    }
    if (allnan) {
      goto exit;
    }
  }

  geom_blob_header_t *spb = &writer->header;
  spb->empty = 0;
  geom_envelope_t *envelope = &spb->envelope;
  geom_envelope_fill(envelope, header, point_count, coords);

exit:
  return result;
}

static int spb_end_geometry(const geom_consumer_t *consumer, const geom_header_t *header, errorstream_t *error) {
  geom_blob_writer_t *writer = (geom_blob_writer_t *) consumer;
  wkb_writer_t *wkb = &writer->wkb_writer;

  geom_consumer_t *wkb_consumer = wkb_writer_geom_consumer(wkb);
  return wkb_consumer->end_geometry(wkb_consumer, header, error);
}

static int spb_end(const geom_consumer_t *consumer, errorstream_t *error) {
  int result = SQLITE_OK;

  geom_blob_writer_t *writer = (geom_blob_writer_t *) consumer;
  wkb_writer_t *wkb = &writer->wkb_writer;
  binstream_t *stream = &wkb->stream;

  size_t pos = binstream_position(stream);
  result = binstream_seek(stream, 0);
  if (result != SQLITE_OK) {
    goto exit;
  }

  geom_envelope_t *envelope = &writer->header.envelope;
  if (geom_envelope_finalize(envelope) == EMPTY_GEOM) {
    writer->header.empty = 1;
  }

  result = spb_write_header(stream, &writer->header, NULL);
  if (result != SQLITE_OK) {
    goto exit;
  }

  result = binstream_seek(stream, pos);
  if (result != SQLITE_OK) {
    goto exit;
  }

  geom_consumer_t *wkb_consumer = wkb_writer_geom_consumer(wkb);
  result = wkb_consumer->end(wkb_consumer, error);

exit:
  return result;
}

int spb_writer_init(geom_blob_writer_t *writer, int32_t srid) {
  geom_consumer_init(&writer->geom_consumer, NULL, spb_end, spb_begin_geometry, spb_end_geometry, spb_coordinates);
  geom_envelope_init(&writer->header.envelope);
  writer->geom_type = GEOM_GEOMETRY;
  writer->header.envelope.has_env_x = 1;
  writer->header.envelope.has_env_y = 1;
  writer->header.srid = srid;
  writer->header.empty = 1;
  return wkb_writer_init(&writer->wkb_writer, WKB_SPATIALITE);
}

void spb_writer_destroy(geom_blob_writer_t *writer, int free_data) {
  wkb_writer_destroy(&writer->wkb_writer, free_data);
}
