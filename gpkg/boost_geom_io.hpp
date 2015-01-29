#ifndef GPKG_BOOST_GEOM_IO_HPP
#define GPKG_BOOST_GEOM_IO_HPP

#include "boost_geometries.hpp"

extern "C" {
#include "geomio.h"
#include "spatialdb_internal.h"
}

/** @private */
typedef enum {
  COORDINATES,
  GEOMETRIES
} boostgeom_data_type_t;

/**
 * A Boost.Geometry writer. boostgeom_writer_t instances can be used to generate a GEOSGeometry object on
 * any geometry source. Use geos_writer_geom_consumer() to obtain a geom_consumer_t pointer that can be passed to
 * geomtery sources.
 */
typedef struct {
  /** @private */
  geom_consumer_t geom_consumer;
  /** @private */
  int srid;
  /** @private */
  gpkg::GeometryPtr geometry;
  /** @private */
  gpkg::GeometryPtr geometry_stack[GEOM_MAX_DEPTH];
  /** @private */
  int child_count[GEOM_MAX_DEPTH];
  /** @private */
  int offset;
} boostgeom_writer_t;

int boostgeom_writer_init_srid(boostgeom_writer_t *writer, int srid);

void boostgeom_writer_destroy(boostgeom_writer_t *writer, int free_data);

geom_consumer_t *boostgeom_writer_geom_consumer(boostgeom_writer_t *writer);

gpkg::GeometryPtr boostgeom_writer_getgeometry(boostgeom_writer_t *writer);

int boostgeom_read_geometry(const gpkg::GeometryPtr geom, geom_consumer_t const *consumer, errorstream_t *error);

#endif
