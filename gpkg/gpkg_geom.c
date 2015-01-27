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
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "gpkg_geom.h"
#include "sqlite.h"
#include "fp.h"
#include "blobio.h"
#include "geomio.h"

#define GPB_VERSION 0
#define GPB_BIG_ENDIAN 0
#define GPB_LITTLE_ENDIAN 1

#define CHECK_ENV_COMP(gpb, comp, error) \
    if (gpb->envelope.has_env_##comp) { \
      if ((gpb->empty && (!fp_isnan(gpb->envelope.min_##comp) || !fp_isnan(gpb->envelope.max_##comp))) || gpb->envelope.min_##comp > gpb->envelope.max_##comp) {\
          if (error) error_append(error, "GPB envelope min" #comp " > max" #comp ": [min: %g, max: %g]", gpb->envelope.min_##comp, gpb->envelope.max_##comp);\
          return SQLITE_IOERR;\
      }\
    }
#define CHECK_ENV(gpb, error) CHECK_ENV_COMP(gpb, x, error) CHECK_ENV_COMP(gpb, y, error) CHECK_ENV_COMP(gpb, z, error) CHECK_ENV_COMP(gpb, m, error)

static size_t gpb_header_size(geom_blob_header_t *gpb) {
  int coord_count = ((gpb->envelope.has_env_x ? 2 : 0) + (gpb->envelope.has_env_y ? 2 : 0) + (gpb->envelope.has_env_z ? 2 : 0) + (gpb->envelope.has_env_m ? 2 : 0));
  return (size_t) 4 /* Magic number */  + 4 /* SRID */ + 8 * coord_count /* Envelope */;
}

int gpb_read_header(binstream_t *stream, geom_blob_header_t *gpb, errorstream_t *error) {
  uint8_t head[2];
  if (binstream_nread_u8(stream, head, 2) != SQLITE_OK) {
    return SQLITE_IOERR;
  }

  if (memcmp(head, "GP", 2) != 0) {
    if (error) {
      error_append(error, "Incorrect GPB magic number [expected: GP, actual:%*s]", 2, head);
    }
    return SQLITE_IOERR;
  }

  if (binstream_read_u8(stream, &gpb->version)) {
    return SQLITE_IOERR;
  }

  if (gpb->version != GPB_VERSION) {
    if (error) {
      error_append(error, "Incorrect GPB version [expected: %d, actual:%d]", GPB_VERSION, gpb->version);
    }
    return SQLITE_IOERR;
  }

  uint8_t flags;
  if (binstream_read_u8(stream, &flags)) {
    return SQLITE_IOERR;
  }

  gpb->empty = (flags >> 4) & 0x1;
  uint8_t envelope = (flags >> 1) & 0x7;
  uint8_t endian = flags & 0x1;

  if (envelope > 4) {
    if (error) {
      error_append(error, "Incorrect GPB envelope value: [expected: [0-4], actual:%u]", envelope);
    }
    return SQLITE_IOERR;
  }

  binstream_set_endianness(stream, endian == GPB_BIG_ENDIAN ? BIG : LITTLE);
  if (binstream_read_i32(stream, &gpb->srid) != SQLITE_OK) {
    return SQLITE_IOERR;
  }

  if (envelope > 0) {
    gpb->envelope.has_env_x = 1;
    if (binstream_read_double(stream, &gpb->envelope.min_x)) {
      return SQLITE_IOERR;
    }
    if (binstream_read_double(stream, &gpb->envelope.max_x)) {
      return SQLITE_IOERR;
    }
    gpb->envelope.has_env_y = 1;
    if (binstream_read_double(stream, &gpb->envelope.min_y)) {
      return SQLITE_IOERR;
    }
    if (binstream_read_double(stream, &gpb->envelope.max_y)) {
      return SQLITE_IOERR;
    }
  } else {
    gpb->envelope.has_env_x = 0;
    gpb->envelope.min_x = 0.0;
    gpb->envelope.max_x = 0.0;
    gpb->envelope.has_env_y = 0;
    gpb->envelope.min_y = 0.0;
    gpb->envelope.max_y = 0.0;
  }

  if (envelope == 2 || envelope == 4) {
    gpb->envelope.has_env_z = 1;
    if (binstream_read_double(stream, &gpb->envelope.min_z)) {
      return SQLITE_IOERR;
    }
    if (binstream_read_double(stream, &gpb->envelope.max_z)) {
      return SQLITE_IOERR;
    }
  } else {
    gpb->envelope.has_env_z = 0;
    gpb->envelope.min_z = 0.0;
    gpb->envelope.max_z = 0.0;
  }

  if (envelope == 3 || envelope == 4) {
    gpb->envelope.has_env_m = 1;
    if (binstream_read_double(stream, &gpb->envelope.min_m)) {
      return SQLITE_IOERR;
    }
    if (binstream_read_double(stream, &gpb->envelope.max_m)) {
      return SQLITE_IOERR;
    }
  } else {
    gpb->envelope.has_env_m = 0;
    gpb->envelope.min_m = 0.0;
    gpb->envelope.max_m = 0.0;
  }

  CHECK_ENV(gpb, error)

  return SQLITE_OK;
}

int gpb_write_header(binstream_t *stream, geom_blob_header_t *gpb, errorstream_t *error) {
  CHECK_ENV(gpb, error)

  if (binstream_write_nu8(stream, (uint8_t *)"GP", 2)) {
    return SQLITE_IOERR;
  }

  if (binstream_write_u8(stream, gpb->version)) {
    return SQLITE_IOERR;
  }

  uint8_t envelope = 0;
  if (gpb->envelope.has_env_x && gpb->envelope.has_env_y) {
    envelope = 1;
    if (gpb->envelope.has_env_z && gpb->envelope.has_env_m) {
      envelope = 4;
    } else if (gpb->envelope.has_env_z) {
      envelope = 2;
    } else if (gpb->envelope.has_env_m) {
      envelope = 3;
    }
  }
  uint8_t endian = binstream_get_endianness(stream) == LITTLE ? 1 : 0;
  uint8_t empty = gpb->empty == 0 ? 0 : 1;

  uint8_t flags = (empty << 4) | (envelope << 1) | endian;
  if (binstream_write_u8(stream, flags)) {
    return SQLITE_IOERR;
  }

  if (binstream_write_i32(stream, gpb->srid)) {
    return SQLITE_IOERR;
  }

  if (gpb->envelope.has_env_x) {
    if (binstream_write_double(stream, gpb->envelope.min_x)) {
      return SQLITE_IOERR;
    }
    if (binstream_write_double(stream, gpb->envelope.max_x)) {
      return SQLITE_IOERR;
    }
  }

  if (gpb->envelope.has_env_y) {
    if (binstream_write_double(stream, gpb->envelope.min_y)) {
      return SQLITE_IOERR;
    }
    if (binstream_write_double(stream, gpb->envelope.max_y)) {
      return SQLITE_IOERR;
    }
  }

  if (gpb->envelope.has_env_z) {
    if (binstream_write_double(stream, gpb->envelope.min_z)) {
      return SQLITE_IOERR;
    }
    if (binstream_write_double(stream, gpb->envelope.max_z)) {
      return SQLITE_IOERR;
    }
  }

  if (gpb->envelope.has_env_m) {
    if (binstream_write_double(stream, gpb->envelope.min_m)) {
      return SQLITE_IOERR;
    }
    if (binstream_write_double(stream, gpb->envelope.max_m)) {
      return SQLITE_IOERR;
    }
  }

  return SQLITE_OK;
}

static int gpb_begin_geometry(const geom_consumer_t *consumer, const geom_header_t *header, errorstream_t *error) {
  int result = SQLITE_OK;

  geom_blob_writer_t *writer = (geom_blob_writer_t *) consumer;
  wkb_writer_t *wkb = &writer->wkb_writer;

  if (wkb->offset < 0) {
    writer->geom_type = header->geom_type;

    geom_blob_header_t *gpb_header = &writer->header;
    if (header->geom_type != GEOM_POINT) {
      geom_envelope_accumulate(&gpb_header->envelope, header);
    }
    result = binstream_relseek(&wkb->stream, (int32_t)gpb_header_size(gpb_header));
    if (result != SQLITE_OK) {
      goto exit;
    }
  }

  geom_consumer_t *wkb_consumer = wkb_writer_geom_consumer(wkb);
  result = wkb_consumer->begin_geometry(wkb_consumer, header, error);
exit:
  return result;
}

static int gpb_coordinates(const geom_consumer_t *consumer, const geom_header_t *header, size_t point_count, const double *coords, int skip_coords, errorstream_t *error) {
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

  geom_blob_header_t *gpb = &writer->header;
  gpb->empty = 0;
  geom_envelope_t *envelope = &gpb->envelope;

  geom_envelope_fill(envelope, header, point_count, coords);

exit:
  return result;
}

static int gpb_end_geometry(const geom_consumer_t *consumer, const geom_header_t *header, errorstream_t *error) {
  geom_blob_writer_t *writer = (geom_blob_writer_t *) consumer;
  wkb_writer_t *wkb = &writer->wkb_writer;

  geom_consumer_t *wkb_consumer = wkb_writer_geom_consumer(wkb);
  return wkb_consumer->end_geometry(wkb_consumer, header, error);
}

static int gpb_end(const geom_consumer_t *consumer, errorstream_t *error) {
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

  result = gpb_write_header(stream, &writer->header, NULL);
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

int gpb_writer_init(geom_blob_writer_t *writer, int32_t srid) {
  geom_consumer_init(&writer->geom_consumer, NULL, gpb_end, gpb_begin_geometry, gpb_end_geometry, gpb_coordinates);
  geom_envelope_init(&writer->header.envelope);
  writer->geom_type = GEOM_GEOMETRY;
  writer->header.version = GPB_VERSION;
  writer->header.srid = srid;
  writer->header.empty = 1;
  return wkb_writer_init(&writer->wkb_writer, WKB_ISO);
}

void gpb_writer_destroy(geom_blob_writer_t *writer, int free_data) {
  wkb_writer_destroy(&writer->wkb_writer, free_data);
}
