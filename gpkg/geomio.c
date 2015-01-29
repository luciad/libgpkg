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
#include <math.h>
#include "sqlite.h"
#include "geomio.h"
#include "fp.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int geom_begin(const geom_consumer_t *consumer, errorstream_t *error) {
  return SQLITE_OK;
}

static int geom_end(const geom_consumer_t *consumer, errorstream_t *error) {
  return SQLITE_OK;
}

static int geom_begin_geometry(const geom_consumer_t *consumer, const geom_header_t *header, errorstream_t *error) {
  return SQLITE_OK;
}

static int geom_end_geometry(const geom_consumer_t *consumer, const geom_header_t *header, errorstream_t *error) {
  return SQLITE_OK;
}

static int geom_coordinates(const geom_consumer_t *consumer, const geom_header_t *header, size_t point_count, const double *coords, int skip_coordinates, errorstream_t *error) {
  return SQLITE_OK;
}

static void intersection2DLSSFCT(double x1, double y1,
                                 double x2, double y2,
                                 double x3, double y3,
                                 double x4, double y4,
                                 double *result) {
  double denom = (y2 - y1) * (x4 - x3) - (x2 - x1) * (y4 - y3);
  result[0] = x1;
  result[1] = y1;
  if (fabs(denom) < 1e-10) {
    result[0] = (x2 + x3) / 2.0;
    result[1] = (y2 + y3) / 2.0;
  } else {
    double s = ((x1 - x3) * (y4 - y3) - (y1 - y3) * (x4 - x3)) / denom;
    result[0] = x1 + s * (x2 - x1);
    result[1] = y1 + s * (y2 - y1);
  }
}

static void find_center_circularArc(double p1_x, double p1_y, double p2_x, double p2_y, double p3_x, double p3_y, double *center) {
  int p1Eqp2 = (p1_x == p2_x && p1_y == p2_y) ? 1 : 0;
  int p1Eqp3 = (p1_x == p3_x && p1_y == p3_y) ? 1 : 0;
  int p2Eqp3 = (p2_x == p3_x && p2_y == p3_y) ? 1 : 0;

  if (p1Eqp2 && p1Eqp3 && p2Eqp3) {
    center[0] = p1_x;
    center[1] = p1_y;
  } else if (p1Eqp2 || p1Eqp3 || p2Eqp3) {
    if (p1Eqp2) {
      center[0] = (p1_x + p3_x) / 2.0;
      center[1] = (p1_y + p3_y) / 2.0;
    } else {
      center[0] = (p1_x + p2_x) / 2.0;
      center[1] = (p1_y + p2_y) / 2.0;
    }
  } else {
    double center1X = (p1_x + p2_x) / 2.0;
    double center1Y = (p1_y + p2_y) / 2.0;
    double center2X = (p2_x + p3_x) / 2.0;
    double center2Y = (p2_y + p3_y) / 2.0;

    double delta1X = p2_x - p1_x;
    double delta1Y = p2_y - p1_y;
    double delta2X = p3_x - p2_x;
    double delta2Y = p3_y - p2_y;

    intersection2DLSSFCT(center1X, center1Y,
                         center1X + delta1Y, center1Y - delta1X,
                         center2X, center2Y,
                         center2X + delta2Y, center2Y - delta2X, center);
  }
}

static double normalize_angle(double angle) {
  if (angle <= -180) {
    return angle + 360.0;
  } else if (angle > 180) {
    return angle - 360.0;
  } else {
    return angle;
  }
}

static int contains_angle(double start_angle, double arc_angle, double target_angle) {
  if (arc_angle >= 360.0 || arc_angle <= -360.0) {
    return 1;
  }
  start_angle = normalize_angle(start_angle);
  double endAngle = start_angle + arc_angle;
  double theta2 = normalize_angle(target_angle);
  if (arc_angle >= 0.0) {
    if ((endAngle > 180.0) && (theta2 < start_angle)) {
      return theta2 + 360 <= endAngle;
    } else {
      return ((theta2 >= start_angle) && (theta2 <= endAngle));
    }
  } else { // aArcAngle < 0
    if ((endAngle <= -180.0) && (theta2 >= start_angle)) {
      return theta2 - 360.0 >= endAngle;
    } else {
      return (theta2 >= endAngle) && (theta2 <= start_angle);
    }
  }
}

static void get_bounds(double center_x, double center_y, double radius, double start_x, double start_y,
                       double end_x, double end_y, double start_angle, double arc_angle, double *bounds) {
  double x_min;
  double y_min;
  double x_max;
  double y_max;

  if (arc_angle >= 360.0 || arc_angle <= -360.0) {
    // We have a full circle, so we can skip the other computations.
    x_min = -radius;
    y_min = -radius;

    x_max = radius;
    y_max = radius;
  } else {
    x_min = fmin(start_x, end_x) - center_x;
    y_min = fmin(start_y, end_y) - center_y;

    x_max = fmax(start_x, end_x) - center_x;
    y_max = fmax(start_y, end_y) - center_y;

    // Should we include the extremal point at angle 0?
    if (contains_angle(start_angle, arc_angle, 0.0)) {
      if (x_min > radius) {
        x_min = radius;
      }
      if (x_max < radius) {
        x_max = radius;
      }
    }

    // Should we include the extremal point at angle 90?
    if (contains_angle(start_angle, arc_angle, 90.0)) {
      if (y_min > radius) {
        y_min = radius;
      }
      if (y_max < radius) {
        y_max = radius;
      }
    }

    // Should we include the extremal point at angle 180?
    if (contains_angle(start_angle, arc_angle, 180.0)) {
      if (x_min > -radius) {
        x_min = -radius;
      }
      if (x_max < -radius) {
        x_max = -radius;
      }
    }

    // Should we include the extremal point at angle 270?
    if (contains_angle(start_angle, arc_angle, 270.0)) {
      if (y_min > -radius) {
        y_min = -radius;
      }
      if (y_max < -radius) {
        y_max = -radius;
      }
    }
  }

  bounds[0] = center_x + x_min;
  bounds[1] = center_y + y_min;
  bounds[2] = bounds[0] + (x_max - x_min);
  bounds[3] = bounds[1] + (y_max - y_min);
}


static double get_radius(double center_x, double center_y, double x, double y) {
  double dx = x - center_x;
  double dy = y - center_y;
  double squaredDistance = dx * dx + dy * dy;
  return sqrt(squaredDistance);
}


static double forward_azimuth2D(double x1, double y1, double x2, double y2) {
  double angle = atan2(y2 - y1, x2 - x1);
  double azimuth = M_PI / 2.0 - angle;
  if (azimuth < 0.0) {
    return azimuth + 2.0 * M_PI;
  } else {
    return azimuth;
  }
}

static double get_angle(double x1, double y1, double x2, double y2) {
  return 90 - forward_azimuth2D(x1, y1, x2, y2) * (180 / M_PI);
}

static double get_arc_angle(double start_angle, double intermediate_angle, double end_angle) {
  if (start_angle < 0) {
    start_angle += 360;
  }
  if (intermediate_angle < 0) {
    intermediate_angle += 360;
  }
  if (end_angle < 0) {
    end_angle += 360;
  }
  double arcAngle = end_angle - start_angle;
  if (start_angle < end_angle) {
    return contains_angle(start_angle, arcAngle, intermediate_angle) ? arcAngle : arcAngle - 360;
  } else {
    return contains_angle(start_angle, arcAngle + 360, intermediate_angle) ? arcAngle + 360 : arcAngle;
  }
}

static void min_max(double value, double *min, double *max) {
  if (value < *min) {
    *min = value;
  }
  if (value > *max) {
    *max = value;
  }
}

static void geom_envelope_fill_arc(geom_envelope_t *envelope, const geom_header_t *header, size_t point_count, const double *coords) {

  double bounds[4];
  double center[2];
  double p1_x, p1_y, p2_x, p2_y, p3_x, p3_y;
#define MIN_MAX(coord, value) do { double coord = value; \
        if (coord < envelope->min_##coord) envelope->min_##coord = coord; \
        if (coord > envelope->max_##coord) envelope->max_##coord = coord; \
      } while(0);

  int offset = 0;
  for (int processed = 0; processed < point_count - 2; processed += 2, offset += header->coord_size * 2) {
    switch (header->coord_type) {
      default:
      case GEOM_XY:
        p1_x = coords[offset];
        p1_y = coords[1 + offset];
        p2_x = coords[2 + offset];
        p2_y = coords[3 + offset];
        p3_x = coords[4 + offset];
        p3_y = coords[5 + offset];
        break;
      case GEOM_XYZ:
        p1_x = coords[offset];
        p1_y = coords[1 + offset];
        p2_x = coords[3 + offset];
        p2_y = coords[4 + offset];
        p3_x = coords[6 + offset];
        p3_y = coords[7 + offset];
        for (int i = 2; i < 9; i += 3) {
          min_max(coords[i + offset], &envelope->min_z, &envelope->max_z);
        }
        break;
      case GEOM_XYM:
        p1_x = coords[offset];
        p1_y = coords[1 + offset];
        p2_x = coords[3 + offset];
        p2_y = coords[4 + offset];
        p3_x = coords[6 + offset];
        p3_y = coords[7 + offset];
        for (int i = 2; i < 9; i += 3) {
          min_max(coords[i + offset], &envelope->min_m, &envelope->max_m);
        }
        break;
      case GEOM_XYZM:
        p1_x = coords[offset];
        p1_y = coords[1 + offset];
        p2_x = coords[4 + offset];
        p2_y = coords[5 + offset];
        p3_x = coords[8 + offset];
        p3_y = coords[9 + offset];
        for (int i = 2; i < 11; i += 4) {
          min_max(coords[i + offset], &envelope->min_z, &envelope->max_z);
          min_max(coords[i + 1 + offset], &envelope->min_m, &envelope->max_m);
        }
    }

    find_center_circularArc(p1_x, p1_y, p2_x, p2_y, p3_x, p3_y, center);
    double radius = get_radius(center[0], center[1], p1_x, p1_y);
    double startAngle = get_angle(center[0], center[1], p1_x, p1_y);
    double intermediateAngle = get_angle(center[0], center[1], p2_x, p2_y);
    double endAngle = get_angle(center[0], center[1], p3_x, p3_y);
    double arcAngle = get_arc_angle(startAngle, intermediateAngle, endAngle);
    get_bounds(center[0], center[1], radius, p1_x, p1_y, p3_x, p3_y, startAngle, arcAngle, bounds);

    min_max(bounds[0], &envelope->min_x, &envelope->max_x);
    min_max(bounds[1], &envelope->min_y, &envelope->max_y);
    min_max(bounds[2], &envelope->min_x, &envelope->max_x);
    min_max(bounds[3], &envelope->min_y, &envelope->max_y);
  }

#undef MIN_MAX
}

static void geom_envelope_fill_simple(geom_envelope_t *envelope, const geom_header_t *header, size_t point_count, const double *coords) {
#define MIN_MAX(coord) double coord = coords[offset++]; \
        if (coord < envelope->min_##coord) envelope->min_##coord = coord; \
        if (coord > envelope->max_##coord) envelope->max_##coord = coord;

  int offset = 0;
  switch (header->coord_type) {
    case GEOM_XY:
      for (size_t i = 0; i < point_count; i++) {
        MIN_MAX(x)
        MIN_MAX(y)
      }
      break;
    case GEOM_XYZ:
      for (size_t i = 0; i < point_count; i++) {
        MIN_MAX(x)
        MIN_MAX(y)
        MIN_MAX(z)
      }
      break;
    case GEOM_XYM:
      for (size_t i = 0; i < point_count; i++) {
        MIN_MAX(x)
        MIN_MAX(y)
        MIN_MAX(m)
      }
      break;
    default:
      for (size_t i = 0; i < point_count; i++) {
        MIN_MAX(x)
        MIN_MAX(y)
        MIN_MAX(z)
        MIN_MAX(m)
      }
  }

#undef MIN_MAX
}

void geom_consumer_init(
  geom_consumer_t *consumer,
  int (*begin)(const geom_consumer_t *, errorstream_t *),
  int (*end)(const geom_consumer_t *, errorstream_t *),
  int (*begin_geometry)(const geom_consumer_t *, const geom_header_t *, errorstream_t *),
  int (*end_geometry)(const geom_consumer_t *, const geom_header_t *, errorstream_t *),
  int (*coordinates)(const geom_consumer_t *, const geom_header_t *, size_t point_count, const double *coords, int skip_coordinates, errorstream_t *)
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
    case GEOM_CURVEPOLYGON:
      *geom_type_name = "CurvePolygon";
      break;
    case GEOM_POLYGON:
      *geom_type_name = "Polygon";
      break;
    case GEOM_GEOMETRYCOLLECTION:
      *geom_type_name = "GeomCollection";
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
    case GEOM_CIRCULARSTRING:
      *geom_type_name = "CircularString";
      break;
    case GEOM_COMPOUNDCURVE:
      *geom_type_name = "CompoundCurve";
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

  // Ignore optional 'ST_' prefix.
  if (sqlite3_strnicmp(type_name, "st_", 3) == 0) {
    type_name += 3;
  }

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
  } else if (sqlite3_strnicmp(type_name, "geom", 4) == 0) {
    const char *remainder = type_name + 4;
    if (sqlite3_strnicmp(remainder, "collection", 11) == 0) {
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
    geom_type = GEOM_CURVEPOLYGON;
  } else if (sqlite3_strnicmp(type_name, "circularstring", 15) == 0) {
    geom_type = GEOM_CIRCULARSTRING;
  } else if (sqlite3_strnicmp(type_name, "compoundcurve", 14) == 0) {
    geom_type = GEOM_COMPOUNDCURVE;
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
    case GEOM_CIRCULARSTRING:
    case GEOM_COMPOUNDCURVE:
    case GEOM_LINEARRING:
      *super_type = GEOM_CURVE;
      break;
    case GEOM_CURVEPOLYGON:
      *super_type = GEOM_SURFACE;
      break;
    case GEOM_POLYGON:
      *super_type = GEOM_CURVEPOLYGON;
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

void geom_envelope_accumulate(geom_envelope_t *envelope, const geom_header_t *header) {
  switch (header->coord_type) {
    case GEOM_XYZ:
      envelope->has_env_x = 1;
      envelope->has_env_y = 1;
      envelope->has_env_z = 1;
      break;
    case GEOM_XYM:
      envelope->has_env_x = 1;
      envelope->has_env_y = 1;
      envelope->has_env_m = 1;
      break;
    case GEOM_XYZM:
      envelope->has_env_x = 1;
      envelope->has_env_y = 1;
      envelope->has_env_z = 1;
      envelope->has_env_m = 1;
      break;
    default:
      envelope->has_env_x = 1;
      envelope->has_env_y = 1;
      break;
  }
}

int geom_envelope_finalize(geom_envelope_t *envelope) {

  if ((envelope->min_x == DBL_MAX && envelope->max_x == -DBL_MAX) ||
      (envelope->min_y == DBL_MAX && envelope->max_y == -DBL_MAX)) {
    double nan = fp_nan();
    envelope->min_x = envelope->max_x = nan;
    envelope->min_y = envelope->max_y = nan;
    envelope->min_z = envelope->max_z = nan;
    envelope->min_m = envelope->max_m = nan;
    return EMPTY_GEOM;
  }

  return 0;
}

void geom_envelope_fill(geom_envelope_t *envelope, const geom_header_t *header, size_t point_count, const double *coords) {
  if (header->geom_type == GEOM_CIRCULARSTRING) {
    geom_envelope_fill_arc(envelope, header, point_count, coords);
  } else {
    geom_envelope_fill_simple(envelope, header, point_count, coords);
  }
}

