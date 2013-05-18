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
#include <string.h>
#include <float.h>
#include "sqlite.h"
#include "geomio.h"

static int geom_begin(const geom_consumer_t *consumer) {
    return SQLITE_OK;
}

static int geom_end(const geom_consumer_t *consumer) {
    return SQLITE_OK;
}

static int geom_begin_geometry(const geom_consumer_t *consumer, const geom_header_t *header) {
    return SQLITE_OK;
}

static int geom_end_geometry(const geom_consumer_t *consumer, const geom_header_t *header) {
    return SQLITE_OK;
}

static int geom_coordinates(const geom_consumer_t *consumer, const geom_header_t *header, size_t point_count, const double *coords) {
    return SQLITE_OK;
}

void geom_consumer_init(
        geom_consumer_t *consumer,
        int (*begin)(const geom_consumer_t *),
        int (*end)(const geom_consumer_t *),
        int (*begin_geometry)(const geom_consumer_t*, const geom_header_t*),
        int (*end_geometry)(const geom_consumer_t*, const geom_header_t*),
        int (*coordinates)(const geom_consumer_t*, const geom_header_t*, size_t point_count, const double* coords)
) {
    consumer->begin = begin != NULL ? begin : geom_begin;
    consumer->end = end != NULL ? end : geom_end;
    consumer->begin_geometry = begin_geometry != NULL ? begin_geometry : geom_begin_geometry;
    consumer->end_geometry = end_geometry != NULL ? end_geometry : geom_end_geometry;
    consumer->coordinates = coordinates != NULL ? coordinates : geom_coordinates;
}

int geom_coord_dim(const geom_header_t *wkb) {
    switch (wkb->coord_type) {
		default:
		case GEOM_XY:
			return 2;
		case GEOM_XYZ:
		case GEOM_XYM:
			return 3;
		case GEOM_XYZM:
			return 4;
	}
}

const char* geom_type_name(const geom_header_t *wkb) {
    switch (wkb->geom_type) {
		case GEOM_POINT:
			return "ST_Point"; 
		case GEOM_LINESTRING:
			return "ST_LineString"; 
		case GEOM_POLYGON:
			return "ST_Polygon";
		case GEOM_MULTIPOINT:
			return "ST_MultiPoint";
		case GEOM_MULTILINESTRING:
			return "ST_MultiLineString";
		case GEOM_MULTIPOLYGON:
			return "ST_MultiPolygon";
		case GEOM_GEOMETRYCOLLECTION:
			return "ST_GeometryCollection";
		default:
			return NULL;
	}
}

void geom_envelope_init(geom_envelope_t *envelope) {
    envelope->has_env_x = 0;
    envelope->min_x = DBL_MAX;
    envelope->max_x = -DBL_MAX;
    envelope->has_env_y = 0;
    envelope->min_y = DBL_MAX;
    envelope->max_y = -DBL_MAX;
    envelope->has_env_z = 0;
    envelope->min_z = DBL_MAX;
    envelope->max_z = -DBL_MAX;
    envelope->has_env_m = 0;
    envelope->min_m = DBL_MAX;
    envelope->max_m = -DBL_MAX;
}
