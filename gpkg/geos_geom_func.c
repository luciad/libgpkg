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
#include <stdio.h>
#include "geos_context.h"
#include "geos_geom_io.h"
#include "geom_func.h"
#include "spatialdb_internal.h"

#define GEOS_START(context) \
  const geos_context_t *geos_context = (const geos_context_t *)sqlite3_user_data(context); \
  char error_buffer[256];\
  error_t error;\
  error_init_fixed(&error, error_buffer, 256)
#define GEOS_CONTEXT geos_context
#define GEOS_HANDLE geos_context->geos_handle
#define GEOS_GET_GEOM(args, i) get_geos_geom( context, geos_context, args[i], &error )
#define GEOS_FREE_GEOM(geom) GEOSGeom_destroy_r( geos_context->geos_handle, geom )

#define GEOS_FUNC1(name) static void ST_##name(sqlite3_context *context, int nbArgs, sqlite3_value **args) {\
  GEOS_START(context);\
  GEOSGeometry *g1 = GEOS_GET_GEOM( args, 0 );\
  if (g1 == NULL) {\
    sqlite3_result_error(context, error_message(&error), -1);\
    return;\
  }\
  char result = GEOS##name##_r(GEOS_HANDLE, g1);\
  if (result == 2) {\
    geom_geos_get_error(&error);\
    sqlite3_result_error(context, error_message(&error), -1);\
  } else {\
    sqlite3_result_int(context, result);\
  }\
  GEOS_FREE_GEOM( g1 );\
}

#define GEOS_FUNC2(name) static void ST_##name(sqlite3_context *context, int nbArgs, sqlite3_value **args) {\
  GEOS_START(context);\
  GEOSGeometry *g1 = GEOS_GET_GEOM( args, 0 );\
  GEOSGeometry *g2 = GEOS_GET_GEOM( args, 1 );\
  if (g1 == NULL || g2 == NULL) {\
    sqlite3_result_error(context, error_message(&error), -1);\
    return;\
  }\
  char result = GEOS##name##_r(GEOS_HANDLE, g1, g2);\
  if (result == 2) {\
    geom_geos_get_error(&error);\
    sqlite3_result_error(context, error_message(&error), -1);\
  } else {\
    sqlite3_result_int(context, result);\
  }\
  GEOS_FREE_GEOM( g1 );\
  GEOS_FREE_GEOM( g2 );\
}

#define GEOS_FUNC1_DBL(name) static void ST_##name(sqlite3_context *context, int nbArgs, sqlite3_value **args) {\
  GEOS_START(context);\
  GEOSGeometry *g1 = GEOS_GET_GEOM( args, 0 );\
  if (g1 == NULL) {\
    sqlite3_result_error(context, error_message(&error), -1);\
    return;\
  }\
  double val;\
  char result = GEOS##name##_r(GEOS_HANDLE, g1, &val);\
  if (result == 1) {\
    sqlite3_result_double(context, val);\
  } else {\
    geom_geos_get_error(&error);\
    sqlite3_result_error(context, error_message(&error), -1);\
  }\
  GEOS_FREE_GEOM( g1 );\
}

#define GEOS_FUNC2_DBL(name) static void ST_##name(sqlite3_context *context, int nbArgs, sqlite3_value **args) {\
  GEOS_START(context);\
  GEOSGeometry *g1 = GEOS_GET_GEOM( args, 0 );\
  GEOSGeometry *g2 = GEOS_GET_GEOM( args, 1 );\
  if (g1 == NULL || g2 == NULL) {\
    sqlite3_result_error(context, error_message(&error), -1);\
    return;\
  }\
  double val;\
  char result = GEOS##name##_r(GEOS_HANDLE, g1, g2, &val);\
  if (result == 1) {\
    sqlite3_result_double(context, val);\
  } else {\
    geom_geos_get_error(&error);\
    sqlite3_result_error(context, error_message(&error), -1);\
  }\
  GEOS_FREE_GEOM( g1 );\
  GEOS_FREE_GEOM( g2 );\
}

#define GEOS_FUNC1_GEOM(name) static void ST_##name(sqlite3_context *context, int nbArgs, sqlite3_value **args) {\
  GEOS_START(context);\
  GEOSGeometry *g1 = GEOS_GET_GEOM( args, 0 );\
  if (g1 == NULL) {\
    sqlite3_result_error(context, error_message(&error), -1);\
    return;\
  }\
  GEOSGeometry *result = GEOS##name##_r(GEOS_HANDLE, g1);\
  if (result != NULL) {\
    set_geos_geom_result(context, GEOS_CONTEXT, result, &error);\
    GEOS_FREE_GEOM( result );\
  } else {\
    geom_geos_get_error(&error);\
    sqlite3_result_error(context, error_message(&error), -1);\
  }\
  GEOS_FREE_GEOM( g1 );\
}

#define GEOS_FUNC2_GEOM(name) static void ST_##name(sqlite3_context *context, int nbArgs, sqlite3_value **args) {\
  GEOS_START(context);\
  GEOSGeometry *g1 = GEOS_GET_GEOM( args, 0 );\
  GEOSGeometry *g2 = GEOS_GET_GEOM( args, 1 );\
  if (g1 == NULL || g2 == NULL) {\
    sqlite3_result_error(context, error_message(&error), -1);\
    return;\
  }\
  GEOSGeometry *result = GEOS##name##_r(GEOS_HANDLE, g1, g2);\
  if (result != NULL) {\
    set_geos_geom_result(context, GEOS_CONTEXT, result, &error);\
    GEOS_FREE_GEOM( result );\
  } else {\
    geom_geos_get_error(&error);\
    sqlite3_result_error(context, error_message(&error), -1);\
  }\
  GEOS_FREE_GEOM( g1 );\
  GEOS_FREE_GEOM( g2 );\
}

typedef struct {
  GEOSContextHandle_t geos_handle;
  const spatialdb_t *spatialdb;
} geos_context_t;

static GEOSGeometry *get_geos_geom(sqlite3_context *context, const geos_context_t *geos_context, sqlite3_value *value, error_t *error) {
  geom_blob_header_t header;

  uint8_t *blob = (uint8_t *)sqlite3_value_blob(value);
  size_t blob_length = (size_t) sqlite3_value_bytes(value);

  binstream_t stream;
  binstream_init(&stream, blob, blob_length);

  geos_writer_t writer;
  geos_writer_init(&writer, geos_context->geos_handle);

  geos_context->spatialdb->read_blob_header(&stream, &header, error);
  geos_context->spatialdb->read_geometry(&stream, geos_writer_geom_consumer(&writer), error);

  GEOSGeometry *g = geos_writer_getgeometry(&writer);
  geos_writer_destroy(&writer, g == NULL);
  return g;
}

static int *set_geos_geom_result(sqlite3_context *context, const geos_context_t *geos_context, GEOSGeometry *geom, error_t *error) {
  geom_blob_writer_t writer;
  geos_context->spatialdb->writer_init_srid( &writer, GEOSGetSRID_r(geos_context->geos_handle, geom) );

  geos_read_geometry(geos_context->geos_handle, geom, geom_blob_writer_geom_consumer(&writer), error);

  sqlite3_result_blob(context, geom_blob_writer_getdata(&writer), geom_blob_writer_length(&writer), sqlite3_free);

  geos_context->spatialdb->writer_destroy( &writer, 0 );

  return SQLITE_OK;
}

GEOS_FUNC1(isSimple)
GEOS_FUNC1(isRing)

#if GEOS_CAPI_VERSION_MINOR >= 7
GEOS_FUNC1(isClosed)
#endif

GEOS_FUNC1(isValid)

GEOS_FUNC2(Disjoint)
GEOS_FUNC2(Touches)
GEOS_FUNC2(Crosses)
GEOS_FUNC2(Within)
GEOS_FUNC2(Contains)
GEOS_FUNC2(Overlaps)
GEOS_FUNC2(Equals)
#if GEOS_CAPI_VERSION_MINOR >= 8
GEOS_FUNC2(Covers)
GEOS_FUNC2(CoveredBy)
#endif

GEOS_FUNC1_DBL(Area)
GEOS_FUNC1_DBL(Length)

GEOS_FUNC2_DBL(Distance)
GEOS_FUNC2_DBL(HausdorffDistance)

GEOS_FUNC1_GEOM(Boundary)
GEOS_FUNC1_GEOM(ConvexHull)
GEOS_FUNC1_GEOM(Envelope)

GEOS_FUNC2_GEOM(Difference)
GEOS_FUNC2_GEOM(SymDifference)
GEOS_FUNC2_GEOM(Intersection)
GEOS_FUNC2_GEOM(Union)

static void geos_context_destroy(void *user_data) {
  geos_context_t *ctx = user_data;
  if (ctx != NULL) {
    geom_geos_destroy(ctx->geos_handle);
    ctx->geos_handle = NULL;
    sqlite3_free(ctx);
  }
}

static void create_geos_function(sqlite3 *db, const char* name, void (*function)(sqlite3_context*,int,sqlite3_value**), int args, const spatialdb_t *spatialdb, error_t *error) {
  geos_context_t *ctx = sqlite3_malloc(sizeof(geos_context_t));
  if (ctx == NULL) {
    error_append(error, "Error allocating GEOS context");
    return;
  }
  GEOSContextHandle_t geos_handle = geom_geos_init();
  if (geos_handle == NULL) {
    sqlite3_free(ctx);
    error_append(error, "Error initializing GEOS");
    return;
  }

  ctx->geos_handle = geos_handle;
  ctx->spatialdb = spatialdb;
  REGISTER_FUNCTION(name, function, args, ctx, geos_context_destroy, error);
}

void geom_func_init(sqlite3 *db, const spatialdb_t *spatialdb, error_t *error) {
  create_geos_function(db, "ST_Area", ST_Area, 1, spatialdb, error);
  create_geos_function(db, "Area", ST_Area, 1, spatialdb, error);

  create_geos_function(db, "ST_Length", ST_Length, 1, spatialdb, error);
  create_geos_function(db, "Length", ST_Length, 1, spatialdb, error);

#if GEOS_CAPI_VERSION_MINOR >= 7
  create_geos_function(db, "ST_isClosed", ST_isClosed, 1, spatialdb, error);
  create_geos_function(db, "isClosed", ST_isClosed, 1, spatialdb, error);
#endif

  create_geos_function(db, "ST_isSimple", ST_isSimple, 1, spatialdb, error);
  create_geos_function(db, "isSimple", ST_isSimple, 1, spatialdb, error);

  create_geos_function(db, "ST_isRing", ST_isRing, 1, spatialdb, error);
  create_geos_function(db, "isRing", ST_isRing, 1, spatialdb, error);

  create_geos_function(db, "ST_isValid", ST_isValid, 1, spatialdb, error);
  create_geos_function(db, "isValid", ST_isValid, 1, spatialdb, error);

  create_geos_function(db, "ST_Disjoint", ST_Disjoint, 2, spatialdb, error);
  create_geos_function(db, "Disjoint", ST_Disjoint, 2, spatialdb, error);

  create_geos_function(db, "ST_Touches", ST_Touches, 2, spatialdb, error);
  create_geos_function(db, "Touches", ST_Touches, 2, spatialdb, error);

  create_geos_function(db, "ST_Crosses", ST_Crosses, 2, spatialdb, error);
  create_geos_function(db, "Crosses", ST_Crosses, 2, spatialdb, error);

  create_geos_function(db, "ST_Within", ST_Within, 2, spatialdb, error);
  create_geos_function(db, "Within", ST_Within, 2, spatialdb, error);

  create_geos_function(db, "ST_Contains", ST_Contains, 2, spatialdb, error);
  create_geos_function(db, "Contains", ST_Contains, 2, spatialdb, error);

  create_geos_function(db, "ST_Overlaps", ST_Overlaps, 2, spatialdb, error);
  create_geos_function(db, "Overlaps", ST_Overlaps, 2, spatialdb, error);

  create_geos_function(db, "ST_Equals", ST_Equals, 2, spatialdb, error);
  create_geos_function(db, "Equals", ST_Equals, 2, spatialdb, error);

#if GEOS_CAPI_VERSION_MINOR >= 8
  create_geos_function(db, "ST_Covers", ST_Covers, 2, spatialdb, error);
  create_geos_function(db, "Covers", ST_Covers, 2, spatialdb, error);

  create_geos_function(db, "ST_CoveredBy", ST_CoveredBy, 2, spatialdb, error);
  create_geos_function(db, "CoveredBy", ST_CoveredBy, 2, spatialdb, error);
#endif

  create_geos_function(db, "ST_Distance", ST_Distance, 2, spatialdb, error);
  create_geos_function(db, "Distance", ST_Distance, 2, spatialdb, error);

  create_geos_function(db, "ST_HausdorffDistance", ST_HausdorffDistance, 2, spatialdb, error);
  create_geos_function(db, "HausdorffDistance", ST_HausdorffDistance, 2, spatialdb, error);

  create_geos_function(db, "ST_Boundary", ST_Boundary, 1, spatialdb, error);
  create_geos_function(db, "Boundary", ST_Boundary, 1, spatialdb, error);

  create_geos_function(db, "ST_ConvexHull", ST_ConvexHull, 1, spatialdb, error);
  create_geos_function(db, "ConvexHull", ST_ConvexHull, 1, spatialdb, error);

  create_geos_function(db, "ST_Envelope", ST_Envelope, 1, spatialdb, error);
  create_geos_function(db, "Envelope", ST_Envelope, 1, spatialdb, error);

  create_geos_function(db, "ST_Difference", ST_Difference, 2, spatialdb, error);
  create_geos_function(db, "Difference", ST_Difference, 2, spatialdb, error);

  create_geos_function(db, "ST_SymDifference", ST_SymDifference, 2, spatialdb, error);
  create_geos_function(db, "SymDifference", ST_SymDifference, 2, spatialdb, error);

  create_geos_function(db, "ST_Intersection", ST_Intersection, 2, spatialdb, error);
  create_geos_function(db, "Intersection", ST_Intersection, 2, spatialdb, error);

  create_geos_function(db, "ST_Union", ST_Union, 2, spatialdb, error);
  create_geos_function(db, "Union", ST_Union, 2, spatialdb, error);
}
