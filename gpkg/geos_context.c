#include <stdarg.h>
#include <stdio.h>
#include "geos.h"
#include "geos_context.h"
#include "sqlite.h"
#include "tls.h"

#if GPKG_GEOM_FUNC == GPKG_GEOS_DL
#include "dynlib.h"
#endif

GPKG_TLS_KEY(last_geos_error)

void geom_geos_clear_error() {
  char *err = GPKG_TLS_GET(last_geos_error);
  if (err != NULL) {
    sqlite3_free(err);
    GPKG_TLS_SET(last_geos_error, NULL);
  }
}

void geom_geos_get_error(errorstream_t *error) {
  char *err = GPKG_TLS_GET(last_geos_error);
  if (err != NULL) {
    error_append(error, err);
    sqlite3_free(err);
    GPKG_TLS_SET(last_geos_error, NULL);
  } else {
    error_append(error, "Unknown GEOS error");
  }
}

static void geom_null_msg_handler(const char *fmt, ...) {
}

static void geom_tls_msg_handler(const char *fmt, ...) {
  geom_geos_clear_error();

  va_list args;
  va_start(args, fmt);
  char *err = sqlite3_vmprintf(fmt, args);
  GPKG_TLS_SET(last_geos_error, err);
  va_end(args);
}

#if GPKG_GEOM_FUNC == GPKG_GEOS
geos_handle_t * geom_geos_init(errorstream_t *error) {
  GPKG_TLS_KEY_CREATE(last_geos_error);
  geos_handle_t *geos = initGEOS_r(geom_null_msg_handler, geom_tls_msg_handler);

  if (geos == NULL) {
    error_append(error, "GEOS initialization failed");
  }

  return geos;
}
#else
geos_handle_t *geom_geos_init(char const *geos_lib, errorstream_t *error) {
  GPKG_TLS_KEY_CREATE(last_geos_error);

  geos_handle_t* handle = NULL;
  void *lib = NULL;

  handle = (geos_handle_t*)sqlite3_malloc(sizeof(geos_handle_t));
  if (handle == NULL) {
    error_append(error, "Could not allocate memory for GEOS handle");
    goto error;
  }

  memset(handle, 0, sizeof(geos_handle_t));

  lib = dynlib_open(geos_lib);
  if (lib == NULL) {
    error_append(error, "Could not open library '%s'", geos_lib);
    goto error;
  }

  handle->geos_lib = lib;
  handle->api.initGEOS_r = (GEOSContextHandle_t (*)(GEOSMessageHandler,GEOSMessageHandler)) dynlib_sym(lib, "initGEOS_r");
  handle->api.finishGEOS_r = (void (*)(GEOSContextHandle_t)) dynlib_sym(lib, "finishGEOS_r");
  handle->api.GEOSversion = (const char * (*)()) dynlib_sym(lib, "GEOSversion");
  handle->api.GEOSisClosed_r = (int (*)(GEOSContextHandle_t,const GEOSGeometry*)) dynlib_sym(lib, "GEOSisClosed_r");
  handle->api.GEOSisEmpty_r = (int (*)(GEOSContextHandle_t,const GEOSGeometry*)) dynlib_sym(lib, "GEOSisEmpty_r");
  handle->api.GEOSisSimple_r = (int (*)(GEOSContextHandle_t,const GEOSGeometry*)) dynlib_sym(lib, "GEOSisSimple_r");
  handle->api.GEOSisRing_r = (int (*)(GEOSContextHandle_t,const GEOSGeometry*)) dynlib_sym(lib, "GEOSisRing_r");
  handle->api.GEOSisValid_r = (int (*)(GEOSContextHandle_t,const GEOSGeometry*)) dynlib_sym(lib, "GEOSisValid_r");
  handle->api.GEOSPreparedCovers_r = (int (*)(GEOSContextHandle_t,const GEOSPreparedGeometry*,const GEOSGeometry*)) dynlib_sym(lib, "GEOSPreparedCovers_r");
  handle->api.GEOSPreparedCoveredBy_r = (int (*)(GEOSContextHandle_t,const GEOSPreparedGeometry*,const GEOSGeometry*)) dynlib_sym(lib, "GEOSPreparedCoveredBy_r");
  handle->api.GEOSPreparedDisjoint_r = (int (*)(GEOSContextHandle_t,const GEOSPreparedGeometry*,const GEOSGeometry*)) dynlib_sym(lib, "GEOSPreparedDisjoint_r");
  handle->api.GEOSPreparedIntersects_r = (int (*)(GEOSContextHandle_t,const GEOSPreparedGeometry*,const GEOSGeometry*)) dynlib_sym(lib, "GEOSPreparedIntersects_r");
  handle->api.GEOSPreparedTouches_r = (int (*)(GEOSContextHandle_t,const GEOSPreparedGeometry*,const GEOSGeometry*)) dynlib_sym(lib, "GEOSPreparedTouches_r");
  handle->api.GEOSPreparedCrosses_r = (int (*)(GEOSContextHandle_t,const GEOSPreparedGeometry*,const GEOSGeometry*)) dynlib_sym(lib, "GEOSPreparedCrosses_r");
  handle->api.GEOSPreparedWithin_r = (int (*)(GEOSContextHandle_t,const GEOSPreparedGeometry*,const GEOSGeometry*)) dynlib_sym(lib, "GEOSPreparedWithin_r");
  handle->api.GEOSPreparedContains_r = (int (*)(GEOSContextHandle_t,const GEOSPreparedGeometry*,const GEOSGeometry*)) dynlib_sym(lib, "GEOSPreparedContains_r");
  handle->api.GEOSPreparedOverlaps_r = (int (*)(GEOSContextHandle_t,const GEOSPreparedGeometry*,const GEOSGeometry*)) dynlib_sym(lib, "GEOSPreparedOverlaps_r");
  handle->api.GEOSEquals_r = (int (*)(GEOSContextHandle_t,const GEOSGeometry*,const GEOSGeometry*)) dynlib_sym(lib, "GEOSEquals_r");
  handle->api.GEOSArea_r = (int (*)(GEOSContextHandle_t,const GEOSGeometry*,double*)) dynlib_sym(lib, "GEOSArea_r");
  handle->api.GEOSLength_r = (int (*)(GEOSContextHandle_t,const GEOSGeometry*,double*)) dynlib_sym(lib, "GEOSLength_r");
  handle->api.GEOSDistance_r = (int (*)(GEOSContextHandle_t,const GEOSGeometry*,const GEOSGeometry*,double*)) dynlib_sym(lib, "GEOSDistance_r");
  handle->api.GEOSHausdorffDistance_r = (int (*)(GEOSContextHandle_t,const GEOSGeometry*,const GEOSGeometry*,double*)) dynlib_sym(lib, "GEOSHausdorffDistance_r");
  handle->api.GEOSBoundary_r = (GEOSGeometry* (*)(GEOSContextHandle_t,const GEOSGeometry*)) dynlib_sym(lib, "GEOSBoundary_r");
  handle->api.GEOSConvexHull_r = (GEOSGeometry* (*)(GEOSContextHandle_t,const GEOSGeometry*)) dynlib_sym(lib, "GEOSConvexHull_r");
  handle->api.GEOSEnvelope_r = (GEOSGeometry* (*)(GEOSContextHandle_t,const GEOSGeometry*)) dynlib_sym(lib, "GEOSEnvelope_r");
  handle->api.GEOSGetCentroid_r = (GEOSGeometry* (*)(GEOSContextHandle_t,const GEOSGeometry*)) dynlib_sym(lib, "GEOSGetCentroid_r");
  handle->api.GEOSGeomGetNumPoints_r = (int (*)(GEOSContextHandle_t,const GEOSGeometry*)) dynlib_sym(lib, "GEOSGeomGetNumPoints_r");
  handle->api.GEOSGeomGetPointN_r = (GEOSGeometry* (*)(GEOSContextHandle_t,const GEOSGeometry*,int)) dynlib_sym(lib, "GEOSGeomGetPointN_r");
  handle->api.GEOSGeomGetStartPoint_r = (GEOSGeometry* (*)(GEOSContextHandle_t,const GEOSGeometry*)) dynlib_sym(lib, "GEOSGeomGetStartPoint_r");
  handle->api.GEOSGeomGetEndPoint_r = (GEOSGeometry* (*)(GEOSContextHandle_t,const GEOSGeometry*)) dynlib_sym(lib, "GEOSGeomGetEndPoint_r");
  handle->api.GEOSGetNumInteriorRings_r = (int (*)(GEOSContextHandle_t,const GEOSGeometry*)) dynlib_sym(lib, "GEOSGetNumInteriorRings_r");
  handle->api.GEOSGetInteriorRingN_r = (GEOSGeometry* (*)(GEOSContextHandle_t,const GEOSGeometry*,int)) dynlib_sym(lib, "GEOSGetInteriorRingN_r");
  handle->api.GEOSGetExteriorRing_r = (GEOSGeometry* (*)(GEOSContextHandle_t,const GEOSGeometry*)) dynlib_sym(lib, "GEOSGetExteriorRing_r");
  handle->api.GEOSGetNumGeometries_r = (int (*)(GEOSContextHandle_t,const GEOSGeometry*)) dynlib_sym(lib, "GEOSGetNumGeometries_r");
  handle->api.GEOSGetGeometryN_r = (GEOSGeometry* (*)(GEOSContextHandle_t,const GEOSGeometry*,int)) dynlib_sym(lib, "GEOSGetGeometryN_r");
  handle->api.GEOSDifference_r = (GEOSGeometry* (*)(GEOSContextHandle_t,const GEOSGeometry*,const GEOSGeometry*)) dynlib_sym(lib, "GEOSDifference_r");
  handle->api.GEOSSymDifference_r = (GEOSGeometry* (*)(GEOSContextHandle_t,const GEOSGeometry*,const GEOSGeometry*)) dynlib_sym(lib, "GEOSSymDifference_r");
  handle->api.GEOSIntersection_r = (GEOSGeometry* (*)(GEOSContextHandle_t,const GEOSGeometry*,const GEOSGeometry*)) dynlib_sym(lib, "GEOSIntersection_r");
  handle->api.GEOSUnion_r = (GEOSGeometry* (*)(GEOSContextHandle_t,const GEOSGeometry*,const GEOSGeometry*)) dynlib_sym(lib, "GEOSUnion_r");
  handle->api.GEOSBuffer_r = (GEOSGeometry* (*)(GEOSContextHandle_t,const GEOSGeometry*,double,int)) dynlib_sym(lib, "GEOSBuffer_r");
  handle->api.GEOSRelatePattern_r = (char (*)(GEOSContextHandle_t,const GEOSGeometry*,const GEOSGeometry*,const char *)) dynlib_sym(lib, "GEOSRelatePattern_r");
  handle->api.GEOSGeomTypeId_r = (int (*)(GEOSContextHandle_t,const GEOSGeometry*)) dynlib_sym(lib, "GEOSGeomTypeId_r");
  handle->api.GEOSGetSRID_r = (int (*)(GEOSContextHandle_t,const GEOSGeometry*)) dynlib_sym(lib, "GEOSGetSRID_r");
  handle->api.GEOSSetSRID_r = (void (*)(GEOSContextHandle_t,const GEOSGeometry*,int)) dynlib_sym(lib, "GEOSSetSRID_r");
  handle->api.GEOSGeom_createPoint_r = (GEOSGeometry* (*)(GEOSContextHandle_t,GEOSCoordSequence*)) dynlib_sym(lib, "GEOSGeom_createPoint_r");
  handle->api.GEOSGeom_createEmptyPoint_r = (GEOSGeometry* (*)(GEOSContextHandle_t)) dynlib_sym(lib, "GEOSGeom_createEmptyPoint_r");
  handle->api.GEOSGeom_createLinearRing_r = (GEOSGeometry* (*)(GEOSContextHandle_t,GEOSCoordSequence*)) dynlib_sym(lib, "GEOSGeom_createLinearRing_r");
  handle->api.GEOSGeom_createLineString_r = (GEOSGeometry* (*)(GEOSContextHandle_t,GEOSCoordSequence*)) dynlib_sym(lib, "GEOSGeom_createLineString_r");
  handle->api.GEOSGeom_createEmptyLineString_r = (GEOSGeometry* (*)(GEOSContextHandle_t)) dynlib_sym(lib, "GEOSGeom_createEmptyLineString_r");
  handle->api.GEOSGeom_createEmptyPolygon_r = (GEOSGeometry* (*)(GEOSContextHandle_t)) dynlib_sym(lib, "GEOSGeom_createEmptyPolygon_r");
  handle->api.GEOSGeom_createPolygon_r = (GEOSGeometry* (*)(GEOSContextHandle_t,GEOSGeometry*,GEOSGeometry**,unsigned int)) dynlib_sym(lib, "GEOSGeom_createPolygon_r");
  handle->api.GEOSGeom_createCollection_r = (GEOSGeometry* (*)(GEOSContextHandle_t,int,GEOSGeometry**,unsigned int)) dynlib_sym(lib, "GEOSGeom_createCollection_r");
  handle->api.GEOSGeom_createEmptyCollection_r = (GEOSGeometry* (*)(GEOSContextHandle_t,int)) dynlib_sym(lib, "GEOSGeom_createEmptyCollection_r");
  handle->api.GEOSGeom_getCoordSeq_r = (const GEOSCoordSequence* (*)(GEOSContextHandle_t,const GEOSGeometry*)) dynlib_sym(lib, "GEOSGeom_getCoordSeq_r");
  handle->api.GEOSCoordSeq_create_r = (GEOSCoordSequence* (*)(GEOSContextHandle_t,unsigned int,unsigned int)) dynlib_sym(lib, "GEOSCoordSeq_create_r");
  handle->api.GEOSCoordSeq_getSize_r = (int (*)(GEOSContextHandle_t,const GEOSCoordSequence*,unsigned int *)) dynlib_sym(lib, "GEOSCoordSeq_getSize_r");
  handle->api.GEOSCoordSeq_getX_r = (int (*)(GEOSContextHandle_t,const GEOSCoordSequence*,unsigned int,double*)) dynlib_sym(lib, "GEOSCoordSeq_getX_r");
  handle->api.GEOSCoordSeq_getY_r = (int (*)(GEOSContextHandle_t,const GEOSCoordSequence*,unsigned int,double*)) dynlib_sym(lib, "GEOSCoordSeq_getY_r");
  handle->api.GEOSCoordSeq_setX_r = (int (*)(GEOSContextHandle_t,GEOSCoordSequence*,unsigned int,double)) dynlib_sym(lib, "GEOSCoordSeq_setX_r");
  handle->api.GEOSCoordSeq_setY_r = (int (*)(GEOSContextHandle_t,GEOSCoordSequence*,unsigned int,double)) dynlib_sym(lib, "GEOSCoordSeq_setY_r");
  handle->api.GEOSPrepare_r = (GEOSPreparedGeometry const *(*)(GEOSContextHandle_t,const GEOSGeometry*)) dynlib_sym(lib, "GEOSPrepare_r");
  handle->api.GEOSPreparedGeom_destroy_r = (void (*)(GEOSContextHandle_t,const GEOSPreparedGeometry*)) dynlib_sym(lib, "GEOSPreparedGeom_destroy_r");
  handle->api.GEOSGeom_destroy_r = (void (*)(GEOSContextHandle_t,const GEOSGeometry*)) dynlib_sym(lib, "GEOSGeom_destroy_r");

  // Only init GEOS if the bare minimum set of functions is available to
  // initialize, cleanup and determine the version number.
  if (handle->api.initGEOS_r && handle->api.finishGEOS_r && handle->api.GEOSversion) {
    handle->context = handle->api.initGEOS_r(geom_null_msg_handler, geom_tls_msg_handler);

    if (handle->context != NULL) {
      return handle;
    } else {
      error_append(error, "GEOS initialization failed");
    }
  } else {
    error_append(error, "Could not load symbols 'initGEOS_r', 'finishGEOS_r' and/or 'GEOSversion' from '%s'", geos_lib);
  }

  error:

  if (handle != NULL) {
    sqlite3_free(handle);
  }

  if (lib != NULL) {
    dynlib_close(lib);
  }

  return NULL;
}
#endif

void geom_geos_destroy(geos_handle_t *geos) {
  if (!geos) {
    return;
  }

#if GPKG_GEOM_FUNC == GPKG_GEOS

  finishGEOS_r(geos);

#else

  geos->api.finishGEOS_r(geos->context);
  dynlib_close(geos->geos_lib);
  sqlite3_free(geos);

#endif
}
