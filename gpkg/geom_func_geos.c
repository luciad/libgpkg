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
#include <geos_c.h>
#include <stdio.h>
#include "geom_func.h"
#include "spatialdb_internal.h"
#include "tls.h"

static void geom_null_msg_handler(const char *fmt, ...) {
}

GPKG_TLS_KEY(last_geos_error)

static void geom_clear_geos_error() {
  char *err = GPKG_TLS_GET(last_geos_error);
  if (err != NULL) {
    sqlite3_free(err);
    GPKG_TLS_SET(last_geos_error, NULL);
  }
}

static void geom_get_geos_error(error_t *error) {
  char *err = GPKG_TLS_GET(last_geos_error);
  if (err != NULL) {
    error_append(error, err);
    sqlite3_free(err);
    GPKG_TLS_SET(last_geos_error, NULL);
  } else {
    error_append(error, "GEOS error");
  }
}

static void geom_tls_msg_handler(const char *fmt, ...) {
  geom_clear_geos_error();

  int result;
  va_list args;
  va_start(args, fmt);
  char *err = sqlite3_vmprintf(fmt, args);
  GPKG_TLS_SET(last_geos_error, err);
  va_end(args);
}

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
    geom_get_geos_error(&error);\
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
    geom_get_geos_error(&error);\
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
    geom_get_geos_error(&error);\
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
    geom_get_geos_error(&error);\
    sqlite3_result_error(context, error_message(&error), -1);\
  }\
  GEOS_FREE_GEOM( g1 );\
  GEOS_FREE_GEOM( g2 );\
}

typedef struct {
  GEOSContextHandle_t geos_handle;
  GEOSWKBReader *wkbreader;
  const spatialdb_t *spatialdb;
} geos_context_t;

static GEOSGeometry *get_geos_geom(sqlite3_context *context, const geos_context_t *geos_context, sqlite3_value *value, error_t *error) {
  geom_blob_header_t header;

  uint8_t *blob = (uint8_t *)sqlite3_value_blob(value);
  size_t blob_length = (size_t) sqlite3_value_bytes(value);

  binstream_t stream;
  binstream_init(&stream, blob, blob_length);

  geos_context->spatialdb->read_blob_header(&stream, &header, error);
  geom_clear_geos_error();
  GEOSGeometry *g = GEOSWKBReader_read_r(geos_context->geos_handle, geos_context->wkbreader, binstream_data(&stream), binstream_available(&stream));
  if (g == NULL) {
    geom_get_geos_error(error);
  }
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
  geos_context_t *ctx = sqlite3_malloc(sizeof(geos_context_t));
  GPKG_TLS_KEY_CREATE(last_geos_error);
  GEOSContextHandle_t geos_handle = initGEOS_r(geom_null_msg_handler, geom_tls_msg_handler);

  ctx->geos_handle = geos_handle;
  ctx->wkbreader = GEOSWKBReader_create_r(geos_handle);
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
