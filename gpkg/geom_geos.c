#include <geos_c.h>
#include <malloc.h>
#include <stdio.h>
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
  char result = GEOS##name##_r(GEOS_HANDLE, g1);\
  if (result == 2) {\
    sqlite3_result_error(context, "GEOS error", -1);\
  } else {\
    sqlite3_result_int(context, result);\
  }\
  GEOS_FREE_GEOM( g1 );\
}

#define GEOS_FUNC2(name) static void ST_##name(sqlite3_context *context, int nbArgs, sqlite3_value **args) {\
  GEOS_START(context);\
  GEOSGeometry *g1 = GEOS_GET_GEOM( args, 0 );\
  GEOSGeometry *g2 = GEOS_GET_GEOM( args, 1 );\
  char result = GEOS##name##_r(GEOS_HANDLE, g1, g2);\
  if (result == 2) {\
    sqlite3_result_error(context, "GEOS error", -1);\
  } else {\
    sqlite3_result_int(context, result);\
  }\
  GEOS_FREE_GEOM( g1 );\
  GEOS_FREE_GEOM( g2 );\
}

#define GEOS_FUNC1_DBL(name) static void ST_##name(sqlite3_context *context, int nbArgs, sqlite3_value **args) {\
  GEOS_START(context);\
  GEOSGeometry *g1 = GEOS_GET_GEOM( args, 0 );\
  double val;\
  char result = GEOS##name##_r(GEOS_HANDLE, g1, &val);\
  if (result == 1) {\
    sqlite3_result_double(context, val);\
  } else {\
    sqlite3_result_error(context, "GEOS error", -1);\
  }\
  GEOS_FREE_GEOM( g1 );\
}

#define GEOS_FUNC2_DBL(name) static void ST_##name(sqlite3_context *context, int nbArgs, sqlite3_value **args) {\
  GEOS_START(context);\
  GEOSGeometry *g1 = GEOS_GET_GEOM( args, 0 );\
  GEOSGeometry *g2 = GEOS_GET_GEOM( args, 1 );\
  double val;\
  char result = GEOS##name##_r(GEOS_HANDLE, g1, g2, &val);\
  if (result == 1) {\
    sqlite3_result_double(context, val);\
  } else {\
    sqlite3_result_error(context, "GEOS error", -1);\
  }\
  GEOS_FREE_GEOM( g1 );\
  GEOS_FREE_GEOM( g2 );\
}

typedef struct {
  GEOSContextHandle_t geos_handle;
  GEOSWKBReader *wkbreader;
  const spatialdb_t *spatialdb;
} geos_context_t;

static GEOSGeometry *get_geos_geom( sqlite3_context *context, const geos_context_t *geos_context, sqlite3_value *value, error_t *error ) {
  geom_blob_header_t header;

  uint8_t *blob = (uint8_t *)sqlite3_value_blob(value);
  size_t blob_length = (size_t) sqlite3_value_bytes(value);

  binstream_t stream;
  binstream_init(&stream, blob, blob_length);

  geos_context->spatialdb->read_blob_header(&stream, &header, error);
  return GEOSWKBReader_read_r(geos_context->geos_handle, geos_context->wkbreader, binstream_data(&stream), binstream_available(&stream));
}

GEOS_FUNC1(isSimple)
GEOS_FUNC1(isRing)

/*
TODO segfaults; to be investigated
GEOS_FUNC1(isClosed)
*/
GEOS_FUNC1(isValid)

GEOS_FUNC2(Disjoint)
GEOS_FUNC2(Touches)
GEOS_FUNC2(Crosses)
GEOS_FUNC2(Within)
GEOS_FUNC2(Contains)
GEOS_FUNC2(Overlaps)
GEOS_FUNC2(Equals)
GEOS_FUNC2(Covers)
GEOS_FUNC2(CoveredBy)

GEOS_FUNC1_DBL(Area)
GEOS_FUNC1_DBL(Length)

GEOS_FUNC2_DBL(Distance)
GEOS_FUNC2_DBL(HausdorffDistance)

void geom_func_init(sqlite3*db, const spatialdb_t *spatialdb, error_t *error) {
  geos_context_t *ctx = malloc(sizeof(geos_context_t));

  GEOSContextHandle_t geos_handle = initGEOS_r(NULL, NULL);

  ctx->geos_handle = geos_handle;
  ctx->wkbreader = GEOSWKBReader_create_r(geos_handle);
  ctx->spatialdb = spatialdb;

  REG_FUNC(ST, Area, 1, ctx, error);
  REG_FUNC(ST, Length, 1, ctx, error);

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
  REG_FUNC(ST, Covers, 2, ctx, error);
  REG_FUNC(ST, CoveredBy, 2, ctx, error);
  
  REG_FUNC(ST, Distance, 2, ctx, error);
  REG_FUNC(ST, HausdorffDistance, 2, ctx, error);
}
