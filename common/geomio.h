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

#include <stdint.h>

/**
 * @addtogroup geomio Geometry I/O
 * @{
 */

/**
 * Enumeration of geometry types.
 */
typedef enum {
    /**
     * Point
     */
    GEOM_POINT,
    /**
     * Line string
     */
    GEOM_LINESTRING,
    /**
     * Polygon
     */
    GEOM_POLYGON,
    /**
     * Multi point
     */
    GEOM_MULTIPOINT,
    /**
     * Multi line string
     */
    GEOM_MULTILINESTRING,
    /**
     * Multi polygon
     */
    GEOM_MULTIPOLYGON,
    /**
     * Geometry collection
     */
    GEOM_GEOMETRYCOLLECTION,
    /**
     * Linear ring. Note that this is not a top level geometry type. It is present to allow linear rings to be treated
     * in the same way as other geometries.
     */
    GEOM_LINEARRING
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
 * The header of a geometry.
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
     * Called at the beginning of each geometry.
     * @param consumer the geometry consumer
     * @param header the geometry header
     */
    void (*begin)(struct geom_consumer_t *consumer, geom_header_t *header);
    /**
     * Called at the end of each geometry.
     * @param consumer the geometry consumer
     * @param header the geometry header
     */
    void (*end)(struct geom_consumer_t *consumer, geom_header_t *header);
    /**
     * Called zero or more times per geometry to pass coordinates to the geometry consumer.
     * @param consumer the geometry consumer
     * @param header the geometry header
     * @param point_count the number of points
     * @param coords the coordinate array. This array contains (point_count * header->coord_size) values.
     */
    void (*coordinates)(struct geom_consumer_t *consumer, geom_header_t *header, size_t point_count, double *coords);
} geom_consumer_t;

/**
 * Initializes a geometry consumer.
 * @param begin the begin callback
 * @param end the end callback
 * @param coordinates the coordinates callback
 */
void geom_consumer_init(
        geom_consumer_t *consumer,
        void (*begin)(geom_consumer_t *, geom_header_t *),
        void (*end)(geom_consumer_t *, geom_header_t *),
        void (*coordinates)(geom_consumer_t *, geom_header_t *, size_t point_count, double *coords)
);

/**
 * Returns the coordinate dimension of the geometry.
 * @param header the geometry header containing the coordinate dimension information
 */
int geom_coord_dim(geom_header_t *header);

/**
 * Returns the geometry type as a string.
 * @param header the geometry header containing the type information
 */
char* geom_type_name(geom_header_t *header);

/**
 * Initializes a geometry envelope.
 * @param envelope the envelope to initialize
 */
void geom_envelope_init(geom_envelope_t *envelope);

/** @} */

#endif
