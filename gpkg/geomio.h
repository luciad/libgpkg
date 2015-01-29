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
#ifndef GPKG_GEOMIO_H
#define GPKG_GEOMIO_H

#include <stddef.h>
#include <stdint.h>
#include "error.h"

#define EMPTY_GEOM 1

/**
 * @addtogroup geomio Geometry I/O
 * @{
 */

/**
 * Enumeration of geometry types.
 */
typedef enum {
  GEOM_GEOMETRY = 0,
  /**
   * Point
   */
  GEOM_POINT = 1,
  /**
   * Line string
   */
  GEOM_LINESTRING = 2,
  /**
   * Polygon
   */
  GEOM_POLYGON = 3,
  /**
   * Multi point
   */
  GEOM_MULTIPOINT = 4,
  /**
   * Multi line string
   */
  GEOM_MULTILINESTRING = 5,
  /**
   * Multi polygon
   */
  GEOM_MULTIPOLYGON = 6,
  /**
   * Geometry collection
   */
  GEOM_GEOMETRYCOLLECTION = 7,
  /**
  * Circular String
  */
  GEOM_CIRCULARSTRING = 8,
  /**
  * Compound curve
  */
  GEOM_COMPOUNDCURVE = 9,
  /**
   * Curve polygon
   */
  GEOM_CURVEPOLYGON = 10,
  /**
   * Multi curve
   */
  GEOM_MULTICURVE = 11,
  /**
   * Multi surface
   */
  GEOM_MULTISURFACE = 12,
  /**
   * Surface
   */
  GEOM_SURFACE = 997,
  /**
   * Curve
   */
  GEOM_CURVE = 998,
  /**
   * Linear ring. Note that this is not a top level geometry type. It is present to allow linear rings to be treated
   * in the same way as other geometries.
   */
  GEOM_LINEARRING = 999,
} geom_type_t;

/**
 * The maximum geometry nesting depth supported by the library.
 */
#define GEOM_MAX_DEPTH 25

/**
 * An enumeration of coordinate types.
 */
typedef enum {
  /**
   * XY coordinates
   */
  GEOM_XY,
  /**
   * XYZ coordinates
   */
  GEOM_XYZ,
  /**
   * XYM coordinates
   */
  GEOM_XYM,
  /**
   * XYZM coordinates
   */
  GEOM_XYZM
} coord_type_t;

/**
 * The maximum number of ordinates per coordinate.
 */
#define GEOM_MAX_COORD_SIZE 4

/**
 * The header of a geometry. All geometries, including top-level and nested geometries, have a geometry header.
 */
typedef struct {
  /**
   * The type of the geometry.
   */
  geom_type_t geom_type;
  /**
   * The type of coordinates used by the geometry.
   */
  coord_type_t coord_type;
  /**
   * The number of ordinates per coordinate.
   */
  uint32_t coord_size;
} geom_header_t;

/**
 * A geometry envelope.
 */
typedef struct {
  /**
   * Indicates if min/max X values are present.
   */
  int has_env_x;
  /**
   * The minimum X coordinate of the geometry. Only valid if has_env_x is not 0.
   */
  double min_x;
  /**
    * The maximum X coordinate of the geometry. Only valid if has_env_x is not 0.
    */
  double max_x;
  /**
    * Indicates if min/max Y values are present.
    */
  int has_env_y;
  /**
   * The minimum Y coordinate of the geometry. Only valid if has_env_y is not 0.
   */
  double min_y;
  /**
    * The maximum Y coordinate of the geometry. Only valid if has_env_y is not 0.
    */
  double max_y;
  /**
    * Indicates if min/max Z values are present.
    */
  int has_env_z;
  /**
   * The minimum Z coordinate of the geometry. Only valid if has_env_z is not 0.
   */
  double min_z;
  /**
    * The maximum Z coordinate of the geometry. Only valid if has_env_z is not 0.
    */
  double max_z;
  /**
    * Indicates if min/max M values are present.
    */
  int has_env_m;
  /**
   * The minimum M coordinate of the geometry. Only valid if has_env_m is not 0.
   */
  double min_m;
  /**
    * The maximum M coordinate of the geometry. Only valid if has_env_m is not 0.
    */
  double max_m;
} geom_envelope_t;

/**
 * A geometry consumer.
 */
typedef struct geom_consumer_t {
  /**
   * Called at the beginning of a root geometry.
   * @param consumer the geometry consumer
   * @return SQLITE_OK or an error code
   */
  int (*begin)(const struct geom_consumer_t *consumer, errorstream_t *error);
  /**
   * Called at the beginning of a root geometry.
   * @param consumer the geometry consumer
   * @return SQLITE_OK or an error code
   */
  int (*end)(const struct geom_consumer_t *consumer, errorstream_t *error);
  /**
   * Called at the beginning of each individual geometry including the root.
   * @param consumer the geometry consumer
   * @param header the geometry header
   * @return SQLITE_OK or an error code
   */
  int (*begin_geometry)(const struct geom_consumer_t *consumer, const geom_header_t *header, errorstream_t *error);
  /**
   * Called at the end of each individual geometry including the root.
   * @param consumer the geometry consumer
   * @param header the geometry header
   * @return SQLITE_OK or an error code
   */
  int (*end_geometry)(const struct geom_consumer_t *consumer, const geom_header_t *header, errorstream_t *error);
  /**
   * Called zero or more times per geometry to pass coordinates to the geometry consumer.
   * @param consumer the geometry consumer
   * @param header the geometry header
   * @param point_count the number of points
   * @param coords the coordinate array. This array contains (point_count * header->coord_size) values.
   * @return SQLITE_OK or an error code
   */
  int (*coordinates)(const struct geom_consumer_t *consumer, const geom_header_t *header, size_t point_count, const double *coords, int skip_coords, errorstream_t *error);
} geom_consumer_t;

/**
 * Initializes a geometry consumer.
 * @param[out] consumer the geometry consumer to initialize
 * @param begin the begin callback
 * @param end the end callback
 * @param begin_geometry the begin_geometry callback
 * @param end_geometry the end_geometry callback
 * @param coordinates the coordinates callback
 */
void geom_consumer_init(
  geom_consumer_t *consumer,
  int (*begin)(const geom_consumer_t *, errorstream_t *),
  int (*end)(const geom_consumer_t *, errorstream_t *),
  int (*begin_geometry)(const geom_consumer_t *, const geom_header_t *, errorstream_t *),
  int (*end_geometry)(const geom_consumer_t *, const geom_header_t *, errorstream_t *),
  int (*coordinates)(const geom_consumer_t *, const geom_header_t *, size_t point_count, const double *coords, int skip_coords, errorstream_t *)
);

/**
 * Returns the coordinate dimension of the geometry.
 * @param coord_type the coordinate type
 * @return the coordinate dimension corresponding to the coordinate type
 */
int geom_coord_dim(coord_type_t coord_type);

/**
 * Returns the geometry type as a string.
 * @param geom_type the geometry type constant
 * @return the geometry type string
 */
int geom_type_name(geom_type_t geom_type, const char **geom_type_name);

/**
 * Returns the coordinate type as a string.
 * @param coord_type the coordinate type constant
 * @param[out] coord_type_name a pointer to a char* value which will be written to
 * @return the coordinate type string
 */
int geom_coord_type_name(coord_type_t coord_type, const char **coord_type_name);

/**
 * Determines the geometry type constant corresponding to the given geometry type name.
 * @param type_name the geometry type name
 * @param[out] type a pointer to a geom_type_t value which will be written to
 * @return SQLITE_OK or an error code if the type_name is invalid
 */
int geom_type_from_string(const char *type_name, geom_type_t *type);

/**
 * Returns a normalized version of the given geometry type name. This function is equivalent to
 * calling geom_type_from_string followed by geom_type_name.
 * @param geom_type_name the geometry type name
 * @return a normalized geometry type name or NULL if the given geometry type name is invalid
 */
int geom_normalized_type_name(const char *geom_type_name, const char **normalized_geom_type_name);

/**
 * Determines if a geometry with type actual_type can be assigned to a variable with type expected_type. A geometry
 * is considered assignable if the expected type is equal to or a super type of the actual type.
 * @param expected_type the expected geometry type
 * @param expected_type the actual geometry type
 * @return 1 if expected_type is equal to or a super type of actual_type
 *         0 otherwise
 */
int geom_is_assignable(geom_type_t expected_type, geom_type_t actual_type);

/**
 * Initializes a geometry envelope.
 * @param envelope the envelope to initialize
 */
void geom_envelope_init(geom_envelope_t *envelope);

/**
 *
 * @param envelope the envelope to set the coordinate types
 * @param header geometry header which defines the coordinate types
 */
void geom_envelope_accumulate(geom_envelope_t *envelope, const geom_header_t *header);

/**
 *
 * @return EMPTY_GEOM (1) if bounds indicate empty, 0 if not empty
 */
int geom_envelope_finalize(geom_envelope_t *envelope);


/**
 *
 * @param envelope
 * @param header
 * @param point_count
 * @param coords
 */
void geom_envelope_fill(geom_envelope_t *envelope, const geom_header_t *header, size_t point_count, const double *coords);

/** @} */

#endif
