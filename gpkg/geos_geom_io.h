#ifndef GPKG_GEOS_GEOM_IO_H
#define GPKG_GEOS_GEOM_IO_H

#include <stdio.h>
#include "geos.h"
#include "geomio.h"
#include "spatialdb_internal.h"
#include "tls.h"

/** @private */
typedef enum {
  COORDINATES,
  GEOMETRIES
} geos_data_type_t;

/** @private */
typedef struct {
  /** @private */
  geos_data_type_t type;
  /** @private */
  void *data;
  /** @private */
  size_t capacity;
  /** @private */
  size_t count;
} geos_data_t;

/**
 * A GEOS Geometry writer. geos_writer_t instances can be used to generate a GEOSGeometry object on
 * any geometry source. Use geos_writer_geom_consumer() to obtain a geom_consumer_t pointer that can be passed to
 * geomtery sources.
 */
typedef struct {
  /** @private */
  geom_consumer_t geom_consumer;
  /** @private */
  geos_handle_t *context;
  /** @private */
  GEOSGeometry *geometry;
  /** @private */
  geos_data_t childData[GEOM_MAX_DEPTH];
  /** @private */
  int offset;
  /** @private */
  int srid;
} geos_writer_t;

int geos_writer_init_srid(geos_writer_t *writer, geos_handle_t *context, int srid);

void geos_writer_destroy(geos_writer_t *writer, int free_data);

geom_consumer_t *geos_writer_geom_consumer(geos_writer_t *writer);

GEOSGeometry *geos_writer_getgeometry(geos_writer_t *writer);

int geos_read_geometry(geos_handle_t *geos, const GEOSGeometry *geom, geom_consumer_t const *consumer, errorstream_t *error);

#endif
