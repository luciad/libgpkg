#include <stdio.h>
#include "geos_context.h"
#include "geos_geom_io.h"

static int geos_begin_geometry(const struct geom_consumer_t *consumer, const geom_header_t *header, errorstream_t *error) {
  int result = SQLITE_OK;

  geos_writer_t *writer = (geos_writer_t *) consumer;

  writer->offset++;
  writer->childData[writer->offset].count = 0;

  switch (header->geom_type) {
    case GEOM_POINT:
    case GEOM_LINEARRING:
    case GEOM_LINESTRING:
      writer->childData[writer->offset].type = COORDINATES;
      writer->childData[writer->offset].data = sqlite3_malloc(2 * 20 * sizeof(double));
      writer->childData[writer->offset].capacity = 20;
      break;
    case GEOM_POLYGON:
    case GEOM_MULTIPOLYGON:
    case GEOM_MULTILINESTRING:
    case GEOM_GEOMETRYCOLLECTION:
    case GEOM_MULTIPOINT:
      writer->childData[writer->offset].type = GEOMETRIES;
      writer->childData[writer->offset].data = sqlite3_malloc(20 * sizeof(GEOSGeometry *));
      writer->childData[writer->offset].capacity = 20;
      break;
    default:
      result = SQLITE_IOERR;
  }

  if (result != SQLITE_OK) {
    error_append(error, "Unsupported geometry type %d", header->geom_type);
  }

  if (writer->childData[writer->offset].data == NULL) {
    result = SQLITE_NOMEM;
  }

  return result;
}

static int geos_add_geometry(geos_writer_t *writer, GEOSGeometry *geom) {
  geos_data_t *childData = &writer->childData[writer->offset];
  if (childData->count == childData->capacity) {
    size_t new_capacity = childData->capacity * 3 / 2;
    void *new_data = sqlite3_realloc(childData->data, new_capacity * sizeof(GEOSGeometry *));
    if (new_data == NULL) {
      return SQLITE_NOMEM;
    }
    childData->data = new_data;
    childData->capacity = new_capacity;
  }

  GEOSGeometry **childGeom = (GEOSGeometry **)childData->data;
  childGeom[childData->count] = geom;
  childData->count++;
  return SQLITE_OK;
}

static int geos_add_coordinate(geos_writer_t *writer, double x, double y) {
  geos_data_t *childData = &writer->childData[writer->offset];
  if (childData->count == childData->capacity) {
    size_t new_capacity = childData->capacity * 3 / 2;
    void *new_data = sqlite3_realloc(childData->data, new_capacity * 2 * sizeof(double));
    if (new_data == NULL) {
      return SQLITE_NOMEM;
    }
    childData->data = new_data;
    childData->capacity = new_capacity;
  }

  double *coords = (double *)childData->data;
  size_t offset = 2 * childData->count;
  coords[offset] = x;
  coords[offset + 1] = y;
  childData->count++;
  return SQLITE_OK;
}

static GEOSCoordSequence *geos_create_coord_seq(geos_writer_t *writer) {
  geos_data_t *childData = &writer->childData[writer->offset];
  size_t childCount = childData->count;

  GEOSCoordSequence *seq = GEOSCoordSeq_create_r(writer->context, childCount, 2);
  if (seq == NULL) {
    return NULL;
  }

  double *coords = (double *)childData->data;
  for (size_t i = 0; i < childCount; i++) {
    GEOSCoordSeq_setX_r(writer->context, seq, i, *coords++);
    GEOSCoordSeq_setY_r(writer->context, seq, i, *coords++);
  }

  return seq;
}

static int geos_end_geometry(const struct geom_consumer_t *consumer, const geom_header_t *header, errorstream_t *error) {
  int result = SQLITE_OK;

  geos_writer_t *writer = (geos_writer_t *) consumer;
  geos_data_t *childData = &writer->childData[writer->offset];

  size_t childCount = childData->count;

  GEOSGeometry *geom = NULL;
  switch (header->geom_type) {
    case GEOM_POINT:
      if (childCount == 0) {
        geom = GEOSGeom_createEmptyPoint_r(writer->context);
      } else {
        GEOSCoordSequence *seq = geos_create_coord_seq(writer);
        if (seq != NULL) {
          geom = GEOSGeom_createPoint_r(writer->context, seq);
        }
      }
      break;
    case GEOM_LINEARRING:
      if (childCount > 0) {
        GEOSCoordSequence *seq = geos_create_coord_seq(writer);
        if (seq != NULL) {
          geom = GEOSGeom_createLinearRing_r(writer->context, seq);
        }
      }
      break;
    case GEOM_LINESTRING:
      if (childCount == 0) {
        geom = GEOSGeom_createEmptyLineString_r(writer->context);
      } else {
        GEOSCoordSequence *seq = geos_create_coord_seq(writer);
        if (seq != NULL) {
          geom = GEOSGeom_createLineString_r(writer->context, seq);
        }
      }
      break;
    case GEOM_POLYGON:
      if (childCount == 0) {
        geom = GEOSGeom_createEmptyPolygon_r(writer->context);
      } else if (childCount == 1) {
        GEOSGeometry **childGeom = (GEOSGeometry **)childData->data;
        geom = GEOSGeom_createPolygon_r(writer->context, *childGeom, NULL, 0);
      } else {
        GEOSGeometry **childGeom = (GEOSGeometry **)childData->data;
        geom = GEOSGeom_createPolygon_r(writer->context, *childGeom, childGeom + 1, childCount - 1);
      }
      break;
    case GEOM_MULTIPOINT:
      if (childCount == 0) {
        geom = GEOSGeom_createEmptyCollection_r(writer->context, GEOS_MULTIPOINT);
      } else {
        GEOSGeometry **childGeom = (GEOSGeometry **)childData->data;
        geom = GEOSGeom_createCollection_r(writer->context, GEOS_MULTIPOINT, childGeom, childCount);
      }
      break;
    case GEOM_MULTILINESTRING:
      if (childCount == 0) {
        geom = GEOSGeom_createEmptyCollection_r(writer->context, GEOS_MULTILINESTRING);
      } else {
        GEOSGeometry **childGeom = (GEOSGeometry **)childData->data;
        geom = GEOSGeom_createCollection_r(writer->context, GEOS_MULTILINESTRING, childGeom, childCount);
      }
      break;
    case GEOM_MULTIPOLYGON:
      if (childCount == 0) {
        geom = GEOSGeom_createEmptyCollection_r(writer->context, GEOS_MULTIPOLYGON);
      } else {
        GEOSGeometry **childGeom = (GEOSGeometry **)childData->data;
        geom = GEOSGeom_createCollection_r(writer->context, GEOS_MULTIPOLYGON, childGeom, childCount);
      }
      break;
    case GEOM_GEOMETRYCOLLECTION:
      if (childCount == 0) {
        geom = GEOSGeom_createEmptyCollection_r(writer->context, GEOS_GEOMETRYCOLLECTION);
      } else {
        GEOSGeometry **childGeom = (GEOSGeometry **)childData->data;
        geom = GEOSGeom_createCollection_r(writer->context, GEOS_GEOMETRYCOLLECTION, childGeom, childCount);
      }
      break;
    default:
      geom = NULL;
      error_append(error, "Invalid geometry type %d", header->geom_type);
  }

  if (geom == NULL) {
    geom_geos_get_error(error);
    result = SQLITE_ERROR;
    if (childData->type == GEOMETRIES) {
      GEOSGeometry **childGeom = (GEOSGeometry **)childData->data;
      for (int i = 0; i < childCount; i++) {
        GEOSGeom_destroy_r(writer->context, childGeom[i]);
      }
    }
  }

  sqlite3_free(childData->data);
  childData->data = NULL;

  writer->offset--;

  if (result == SQLITE_OK) {
    GEOSSetSRID_r(writer->context, geom, writer->srid);

    if (writer->offset < 0) {
      writer->geometry = geom;
    } else {
      result = geos_add_geometry(writer, geom);
    }
  }

  return result;
}

static int geos_coordinates(const struct geom_consumer_t *consumer, const geom_header_t *header, size_t point_count, const double *coords, int skip_coords, errorstream_t *error) {
  int result = SQLITE_OK;
  geos_writer_t *writer = (geos_writer_t *) consumer;

  size_t offset = 0;
  for (int i = 0; result == SQLITE_OK && i < point_count; i++) {
    result = geos_add_coordinate(writer, coords[offset], coords[offset + 1]);
    offset += header->coord_size;
  }

  return result;
}

int geos_writer_init_srid(geos_writer_t *writer, geos_handle_t *context, int srid) {
  geom_consumer_init(&writer->geom_consumer, NULL, NULL, geos_begin_geometry, geos_end_geometry, geos_coordinates);
  writer->context = context;
  writer->geometry = NULL;
  memset(writer->childData, 0, GEOM_MAX_DEPTH * sizeof(geos_data_t));

  writer->offset = -1;
  writer->srid = srid;

  return SQLITE_OK;
}

void geos_writer_destroy(geos_writer_t *writer, int free_data) {
  if (writer == NULL) {
    return;
  }

  if (free_data && writer->geometry != NULL) {
    GEOSGeom_destroy_r(writer->context, writer->geometry);
    writer->geometry = NULL;
  }

  for (int i = 0; i < GEOM_MAX_DEPTH; i++) {
    geos_data_t *childData = &writer->childData[i];
    if (childData->data != NULL) {
      if (childData->type == GEOMETRIES) {
        GEOSGeometry **childGeom = (GEOSGeometry **)childData->data;
        for (int i = 0; i < childData->count; i++) {
          GEOSGeom_destroy_r(writer->context, childGeom[i]);
        }
      }
      sqlite3_free(childData->data);
      childData->data = NULL;
    }
  }
}

geom_consumer_t *geos_writer_geom_consumer(geos_writer_t *writer) {
  return &writer->geom_consumer;
}

GEOSGeometry *geos_writer_getgeometry(geos_writer_t *writer) {
  return writer->geometry;
}

#define COORD_BATCH_SIZE 10

static int read_geos_coordseq(geos_handle_t *geos, geom_header_t *header, const GEOSCoordSequence *coordseq, geom_consumer_t const *consumer, errorstream_t *error) {
  int result = SQLITE_OK;
  double coord[2 * COORD_BATCH_SIZE];
  uint32_t remaining;
  GEOSCoordSeq_getSize_r(geos, coordseq, &remaining);

  while (remaining > 0) {
    uint32_t points_to_read = (remaining > COORD_BATCH_SIZE ? COORD_BATCH_SIZE : remaining);
    for (int i = 0, ix = 0; i < points_to_read; i++, ix += 2) {
      GEOSCoordSeq_getX_r(geos, coordseq, i, &coord[ix]);
      GEOSCoordSeq_getY_r(geos, coordseq, i, &coord[ix + 1]);
    }

    result = consumer->coordinates(consumer, header, points_to_read, coord, 0, error);
    if (result != SQLITE_OK) {
      return result;
    }

    remaining -= points_to_read;
  }

  return result;
}

static int read_geos_point(geos_handle_t *geos, const GEOSGeometry *geom, geom_consumer_t const *consumer, errorstream_t *error) {
  int result = SQLITE_OK;
  geom_header_t header = {
    GEOM_POINT,
    GEOM_XY,
    2
  };

  if (result == SQLITE_OK) {
    result = consumer->begin_geometry(consumer, &header, error);
  }
  if (result == SQLITE_OK) {
    result = read_geos_coordseq(geos, &header, GEOSGeom_getCoordSeq_r(geos, geom), consumer, error);
  }
  if (result == SQLITE_OK) {
    result = consumer->end_geometry(consumer, &header, error);
  }
  return result;
}

static int read_geos_linestring(geos_handle_t *geos, const GEOSGeometry *geom, geom_consumer_t const *consumer, errorstream_t *error) {
  int result = SQLITE_OK;
  geom_header_t header = {
    GEOM_LINESTRING,
    GEOM_XY,
    2
  };

  if (result == SQLITE_OK) {
    result = consumer->begin_geometry(consumer, &header, error);
  }
  if (result == SQLITE_OK) {
    result = read_geos_coordseq(geos, &header, GEOSGeom_getCoordSeq_r(geos, geom), consumer, error);
  }
  if (result == SQLITE_OK) {
    result = consumer->end_geometry(consumer, &header, error);
  }
  return result;
}

static int read_geos_linearring(geos_handle_t *geos, const GEOSGeometry *geom, geom_consumer_t const *consumer, errorstream_t *error) {
  int result = SQLITE_OK;
  geom_header_t header = {
    GEOM_LINEARRING,
    GEOM_XY,
    2
  };

  if (result == SQLITE_OK) {
    result = consumer->begin_geometry(consumer, &header, error);
  }
  if (result == SQLITE_OK) {
    result = read_geos_coordseq(geos, &header, GEOSGeom_getCoordSeq_r(geos, geom), consumer, error);
  }
  if (result == SQLITE_OK) {
    result = consumer->end_geometry(consumer, &header, error);
  }
  return result;
}

static int read_geos_polygon(geos_handle_t *geos, const GEOSGeometry *geom, geom_consumer_t const *consumer, errorstream_t *error) {
  int result = SQLITE_OK;
  geom_header_t header = {
    GEOM_POLYGON,
    GEOM_XY,
    2
  };

  if (result == SQLITE_OK) {
    result = consumer->begin_geometry(consumer, &header, error);
  }
  if (result == SQLITE_OK && !GEOSisEmpty_r(geos, geom)) {
    result = read_geos_linearring(geos, GEOSGetExteriorRing_r(geos, geom), consumer, error);
    int ring_count = GEOSGetNumInteriorRings_r(geos, geom);
    for (int i = 0; i < ring_count; i++) {
      if (result != SQLITE_OK) {
        break;
      }
      result = read_geos_linearring(geos, GEOSGetInteriorRingN_r(geos, geom, i), consumer, error);
    }
  }

  if (result == SQLITE_OK) {
    result = consumer->end_geometry(consumer, &header, error);
  }

  return result;
}

static int read_geos_multipoint(geos_handle_t *geos, const GEOSGeometry *geom, geom_consumer_t const *consumer, errorstream_t *error) {
  int result = SQLITE_OK;
  geom_header_t header = {
    GEOM_MULTIPOINT,
    GEOM_XY,
    2
  };

  if (result == SQLITE_OK) {
    result = consumer->begin_geometry(consumer, &header, error);
  }
  int point_count = GEOSGetNumGeometries_r(geos, geom);
  for (int i = 0; i < point_count; i++) {
    if (result != SQLITE_OK) {
      break;
    }
    result = read_geos_point(geos, GEOSGetGeometryN_r(geos, geom, i), consumer, error);
  }

  if (result == SQLITE_OK) {
    result = consumer->end_geometry(consumer, &header, error);
  }

  return result;
}

static int read_geos_multilinestring(geos_handle_t *geos, const GEOSGeometry *geom, geom_consumer_t const *consumer, errorstream_t *error) {
  int result = SQLITE_OK;
  geom_header_t header = {
    GEOM_MULTILINESTRING,
    GEOM_XY,
    2
  };

  if (result == SQLITE_OK) {
    result = consumer->begin_geometry(consumer, &header, error);
  }

  int point_count = GEOSGetNumGeometries_r(geos, geom);
  for (int i = 0; i < point_count; i++) {
    if (result != SQLITE_OK) {
      break;
    }
    result = read_geos_linestring(geos, GEOSGetGeometryN_r(geos, geom, i), consumer, error);
  }

  if (result == SQLITE_OK) {
    result = consumer->end_geometry(consumer, &header, error);
  }

  return result;
}

static int read_geos_multipolygon(geos_handle_t *geos, const GEOSGeometry *geom, geom_consumer_t const *consumer, errorstream_t *error) {
  int result = SQLITE_OK;
  geom_header_t header = {
    GEOM_MULTIPOLYGON,
    GEOM_XY,
    2
  };

  if (result == SQLITE_OK) {
    result = consumer->begin_geometry(consumer, &header, error);
  }

  int point_count = GEOSGetNumGeometries_r(geos, geom);
  for (int i = 0; i < point_count; i++) {
    if (result != SQLITE_OK) {
      break;
    }
    result = read_geos_polygon(geos, GEOSGetGeometryN_r(geos, geom, i), consumer, error);
  }

  if (result == SQLITE_OK) {
    result = consumer->end_geometry(consumer, &header, error);
  }

  return result;
}

static int read_geos_geometry(geos_handle_t *geos, const GEOSGeometry *geom, geom_consumer_t const *consumer, errorstream_t *error);

static int read_geos_geometrycollection(geos_handle_t *geos, const GEOSGeometry *geom, geom_consumer_t const *consumer, errorstream_t *error) {
  int result = SQLITE_OK;
  geom_header_t header = {
    GEOM_GEOMETRYCOLLECTION,
    GEOM_XY,
    2
  };

  if (result == SQLITE_OK) {
    result = consumer->begin_geometry(consumer, &header, error);
  }

  int point_count = GEOSGetNumGeometries_r(geos, geom);
  for (int i = 0; i < point_count; i++) {
    if (result != SQLITE_OK) {
      break;
    }
    result = read_geos_geometry(geos, GEOSGetGeometryN_r(geos, geom, i), consumer, error);
  }

  if (result == SQLITE_OK) {
    result = consumer->end_geometry(consumer, &header, error);
  }

  return result;
}

static int read_geos_geometry(geos_handle_t *geos, const GEOSGeometry *geom, geom_consumer_t const *consumer, errorstream_t *error) {
  int type = GEOSGeomTypeId_r(geos, geom);
  if (type == GEOS_POINT) {
    return read_geos_point(geos, geom, consumer, error);
  } else if (type == GEOS_LINESTRING) {
    return read_geos_linestring(geos, geom, consumer, error);
  } else if (type == GEOS_LINEARRING) {
    return read_geos_linearring(geos, geom, consumer, error);
  } else if (type == GEOS_POLYGON) {
    return read_geos_polygon(geos, geom, consumer, error);
  } else if (type == GEOS_MULTIPOINT) {
    return read_geos_multipoint(geos, geom, consumer, error);
  } else if (type == GEOS_MULTILINESTRING) {
    return read_geos_multilinestring(geos, geom, consumer, error);
  } else if (type == GEOS_MULTIPOLYGON) {
    return read_geos_multipolygon(geos, geom, consumer, error);
  } else if (type == GEOS_GEOMETRYCOLLECTION) {
    return read_geos_geometrycollection(geos, geom, consumer, error);
  } else {
    return SQLITE_ERROR;
  }
}

int geos_read_geometry(geos_handle_t *geos, const GEOSGeometry *geom, geom_consumer_t const *consumer, errorstream_t *error) {
  int result;

  result = consumer->begin(consumer, error);
  if (result != SQLITE_OK) {
    goto exit;
  }

  result = read_geos_geometry(geos, geom, consumer, error);
  if (result != SQLITE_OK) {
    goto exit;
  }

  result = consumer->end(consumer, error);
  if (result != SQLITE_OK) {
    goto exit;
  }

exit:
  return result;
}
