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
#include <stdio.h>
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
  int (*begin_geometry)(const geom_consumer_t *, const geom_header_t *),
  int (*end_geometry)(const geom_consumer_t *, const geom_header_t *),
  int (*coordinates)(const geom_consumer_t *, const geom_header_t *, size_t point_count, const double *coords)
) {
  consumer->begin = begin != NULL ? begin : geom_begin;
  consumer->end = end != NULL ? end : geom_end;
  consumer->begin_geometry = begin_geometry != NULL ? begin_geometry : geom_begin_geometry;
  consumer->end_geometry = end_geometry != NULL ? end_geometry : geom_end_geometry;
  consumer->coordinates = coordinates != NULL ? coordinates : geom_coordinates;
}

int geom_coord_dim(coord_type_t coord_type) {
  switch (coord_type) {
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

int geom_normalized_type_name(const char *geom_type, const char **normalized_geom_type) {
  geom_type_t geom_type_enum;

  int res = geom_type_from_string(geom_type, &geom_type_enum);
  if (res != SQLITE_OK) {
    return res;
  }

  return geom_type_name(geom_type_enum, normalized_geom_type);
}

int geom_type_name(geom_type_t geom_type, const char **geom_type_name) {
  int result = SQLITE_OK;

  switch (geom_type) {
    case GEOM_GEOMETRY:
      *geom_type_name = "Geometry";
      break;
    case GEOM_POINT:
      *geom_type_name = "Point";
      break;
    case GEOM_CURVE:
      *geom_type_name = "Curve";
      break;
    case GEOM_LINESTRING:
      *geom_type_name = "LineString";
      break;
    case GEOM_SURFACE:
      *geom_type_name = "Surface";
      break;
    case GEOM_CURVE_POLYGON:
      *geom_type_name = "CurvePolygon";
      break;
    case GEOM_POLYGON:
      *geom_type_name = "Polygon";
      break;
    case GEOM_GEOMETRYCOLLECTION:
      *geom_type_name = "GeometryCollection";
      break;
    case GEOM_MULTISURFACE:
      *geom_type_name = "MultiSurface";
      break;
    case GEOM_MULTIPOLYGON:
      *geom_type_name = "MultiPolygon";
      break;
    case GEOM_MULTICURVE:
      *geom_type_name = "MultiCurve";
      break;
    case GEOM_MULTILINESTRING:
      *geom_type_name = "MultiLineString";
      break;
    case GEOM_MULTIPOINT:
      *geom_type_name = "MultiPoint";
      break;
    default:
      *geom_type_name = NULL;
      result = SQLITE_ERROR;
  }

  return result;
}

int geom_coord_type_name(coord_type_t coord_type, const char **coord_type_name) {
    int result = SQLITE_OK;

    switch (coord_type) {
        case GEOM_XY:
            *coord_type_name = "XY";
            break;
        case GEOM_XYZ:
            *coord_type_name = "XYZ";
            break;
        case GEOM_XYM:
            *coord_type_name = "XYM";
            break;
        case GEOM_XYZM:
            *coord_type_name = "XYZM";
            break;
        default:
            *coord_type_name = NULL;
            result = SQLITE_ERROR;
    }

    return result;
}

int geom_type_from_string(const char *type_name, geom_type_t *type) {
  geom_type_t geom_type = GEOM_GEOMETRY;

  int result = SQLITE_OK;
  if (sqlite3_strnicmp(type_name, "po", 2) == 0) {
    const char *remainder = type_name + 2;
    if (sqlite3_strnicmp(remainder, "int", 4) == 0) {
      geom_type = GEOM_POINT;
    } else if (sqlite3_strnicmp(remainder, "lygon", 6) == 0) {
      geom_type = GEOM_POLYGON;
    } else {
      result = SQLITE_ERROR;
    }
  } else if (sqlite3_strnicmp(type_name, "multi", 5) == 0) {
    const char *remainder = type_name + 5;
    if (sqlite3_strnicmp(remainder, "curve", 6) == 0) {
      geom_type = GEOM_MULTICURVE;
    } else if (sqlite3_strnicmp(remainder, "surface", 8) == 0) {
      geom_type = GEOM_MULTISURFACE;
    } else if (sqlite3_strnicmp(remainder, "linestring", 11) == 0) {
      geom_type = GEOM_MULTILINESTRING;
    } else if (sqlite3_strnicmp(remainder, "po", 2) == 0) {
      remainder = remainder + 2;
      if (sqlite3_strnicmp(remainder, "int", 4) == 0) {
        geom_type = GEOM_MULTIPOINT;
      } else if (sqlite3_strnicmp(remainder, "lygon", 6) == 0) {
        geom_type = GEOM_MULTIPOLYGON;
      } else {
        result = SQLITE_ERROR;
      }
    } else {
      result = SQLITE_ERROR;
    }
  } else if (sqlite3_strnicmp(type_name, "geometry", 8) == 0) {
    const char *remainder = type_name + 8;
    if (sqlite3_strnicmp(remainder, "", 1) == 0) {
      geom_type = GEOM_GEOMETRY;
    } else if (sqlite3_strnicmp(remainder, "collection", 11) == 0) {
      geom_type = GEOM_GEOMETRYCOLLECTION;
    } else {
      result = SQLITE_ERROR;
    }
  } else if (sqlite3_strnicmp(type_name, "curve", 6) == 0) {
    geom_type = GEOM_CURVE;
  } else if (sqlite3_strnicmp(type_name, "surface", 8) == 0) {
    geom_type = GEOM_SURFACE;
  } else if (sqlite3_strnicmp(type_name, "linestring", 11) == 0) {
    geom_type = GEOM_LINESTRING;
  } else if (sqlite3_strnicmp(type_name, "curvepolygon", 13) == 0) {
    geom_type = GEOM_CURVE_POLYGON;
  } else {
    result = SQLITE_ERROR;
  }

  if (result == SQLITE_OK && type != NULL) {
    *type = geom_type;
  }

  return result;
}

static int geom_parent_type(geom_type_t type, geom_type_t *super_type) {
  int result = SQLITE_OK;

  switch (type) {
    default:
    case GEOM_GEOMETRY:
      result = SQLITE_ERROR;
      break;
    case GEOM_POINT:
    case GEOM_CURVE:
    case GEOM_SURFACE:
    case GEOM_GEOMETRYCOLLECTION:
      *super_type = GEOM_GEOMETRY;
      break;
    case GEOM_LINESTRING:
    case GEOM_LINEARRING:
      *super_type = GEOM_CURVE;
      break;
    case GEOM_CURVE_POLYGON:
      *super_type = GEOM_SURFACE;
      break;
    case GEOM_POLYGON:
      *super_type = GEOM_CURVE_POLYGON;
      break;
    case GEOM_MULTISURFACE:
    case GEOM_MULTICURVE:
    case GEOM_MULTIPOINT:
      *super_type = GEOM_GEOMETRYCOLLECTION;
      break;
    case GEOM_MULTIPOLYGON:
      *super_type = GEOM_MULTISURFACE;
      break;
    case GEOM_MULTILINESTRING:
      *super_type = GEOM_MULTICURVE;
      break;
  }

  return result;
}

int geom_is_assignable(geom_type_t expected_type, geom_type_t actual_type) {
  geom_type_t type = actual_type;
  while (1) {
    if (expected_type == type) {
      return 1;
    } else {
      geom_type_t parent_type;
      int res = geom_parent_type(type, &parent_type);
      if (res != SQLITE_OK) {
        return 0;
      } else {
        type = parent_type;
      }
    }
  }

  return 0;
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
