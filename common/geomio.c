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
#include "geomio.h"

void geom_consumer_init(
        geom_consumer_t *consumer,
        void (*begin)(struct geom_consumer_t*, geom_header_t*),
        void (*end)(struct geom_consumer_t*, geom_header_t*),
        void (*coordinates)(struct geom_consumer_t*, geom_header_t*, size_t point_count, double* coords)
) {
    consumer->begin = begin;
    consumer->end = end;
    consumer->coordinates = coordinates;
}

int geom_coord_dim(geom_header_t *wkb) {
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

char* geom_type_name(geom_header_t *wkb) {
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
