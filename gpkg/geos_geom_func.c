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

  geos_writer_destroy(&writer, 0);

  return g;
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

void geom_func_init(sqlite3 *db, const spatialdb_t *spatialdb, error_t *error) {
  printf("geom_func_init\n");
  geos_context_t *ctx = sqlite3_malloc(sizeof(geos_context_t));
  GEOSContextHandle_t geos_handle = geom_geos_init();

  ctx->geos_handle = geos_handle;
  ctx->spatialdb = spatialdb;

  REG_FUNC(ST, Area, 1, ctx, error);
  REG_FUNC(ST, Length, 1, ctx, error);

#if GEOS_CAPI_VERSION_MINOR >= 7
  REG_FUNC(ST, isClosed, 1, ctx, error);
#endif

  REG_FUNC(ST, isSimple, 1, ctx, error);
  REG_FUNC(ST, isRing, 1, ctx, error);
  REG_FUNC(ST, isValid, 1, ctx, error);

  REG_FUNC(ST, Disjoint, 2, ctx, error);
  REG_FUNC(ST, Touches, 2, ctx, error);
  REG_FUNC(ST, Crosses, 2, ctx, error);
  REG_FUNC(ST, Within, 2, ctx, error);
  REG_FUNC(ST, Contains, 2, ctx, error);
  REG_FUNC(ST, Overlaps, 2, ctx, error);
  REG_FUNC(ST, Equals, 2, ctx, error);

#if GEOS_CAPI_VERSION_MINOR >= 8
  REG_FUNC(ST, Covers, 2, ctx, error);
  REG_FUNC(ST, CoveredBy, 2, ctx, error);
#endif

  REG_FUNC(ST, Distance, 2, ctx, error);
  REG_FUNC(ST, HausdorffDistance, 2, ctx, error);
}
