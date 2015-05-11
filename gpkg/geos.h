#ifndef GPKG_GEOS_H
#define GPKG_GEOS_H

#define GPKG_GEOS 1
#define GPKG_GEOS_DL 2

#include "config.h"

#if GPKG_GEOM_FUNC == GPKG_GEOS

#include <geos_c.h>

typedef struct GEOSContextHandle_HS geos_handle_t;

#define GEOSversion(handle) GEOSversion()

#else

enum GEOSGeomTypes {
    GEOS_POINT,
    GEOS_LINESTRING,
    GEOS_LINEARRING,
    GEOS_POLYGON,
    GEOS_MULTIPOINT,
    GEOS_MULTILINESTRING,
    GEOS_MULTIPOLYGON,
    GEOS_GEOMETRYCOLLECTION
};

typedef struct GEOSGeom_t GEOSGeometry;
typedef struct GEOSPrepGeom_t GEOSPreparedGeometry;
typedef struct GEOSCoordSeq_t GEOSCoordSequence;
typedef struct GEOSContextHandle_HS *GEOSContextHandle_t;
typedef void (*GEOSMessageHandler)(const char *fmt, ...);

typedef struct {
    GEOSContextHandle_t (*initGEOS_r)(GEOSMessageHandler,GEOSMessageHandler);
    void (*finishGEOS_r)(GEOSContextHandle_t);
    const char * (*GEOSversion)();
    int (*GEOSisClosed_r)(GEOSContextHandle_t,const GEOSGeometry*);
    int (*GEOSisEmpty_r)(GEOSContextHandle_t,const GEOSGeometry*);
    int (*GEOSisSimple_r)(GEOSContextHandle_t,const GEOSGeometry*);
    int (*GEOSisRing_r)(GEOSContextHandle_t,const GEOSGeometry*);
    int (*GEOSisValid_r)(GEOSContextHandle_t,const GEOSGeometry*);
    int (*GEOSPreparedCovers_r)(GEOSContextHandle_t,const GEOSPreparedGeometry*,const GEOSGeometry*);
    int (*GEOSPreparedCoveredBy_r)(GEOSContextHandle_t,const GEOSPreparedGeometry*,const GEOSGeometry*);
    int (*GEOSPreparedDisjoint_r)(GEOSContextHandle_t,const GEOSPreparedGeometry*,const GEOSGeometry*);
    int (*GEOSPreparedIntersects_r)(GEOSContextHandle_t,const GEOSPreparedGeometry*,const GEOSGeometry*);
    int (*GEOSPreparedTouches_r)(GEOSContextHandle_t,const GEOSPreparedGeometry*,const GEOSGeometry*);
    int (*GEOSPreparedCrosses_r)(GEOSContextHandle_t,const GEOSPreparedGeometry*,const GEOSGeometry*);
    int (*GEOSPreparedWithin_r)(GEOSContextHandle_t,const GEOSPreparedGeometry*,const GEOSGeometry*);
    int (*GEOSPreparedContains_r)(GEOSContextHandle_t,const GEOSPreparedGeometry*,const GEOSGeometry*);
    int (*GEOSPreparedOverlaps_r)(GEOSContextHandle_t,const GEOSPreparedGeometry*,const GEOSGeometry*);
    int (*GEOSEquals_r)(GEOSContextHandle_t,const GEOSGeometry*,const GEOSGeometry*);
    int (*GEOSArea_r)(GEOSContextHandle_t,const GEOSGeometry*,double*);
    int (*GEOSLength_r)(GEOSContextHandle_t,const GEOSGeometry*,double*);
    int (*GEOSDistance_r)(GEOSContextHandle_t,const GEOSGeometry*,const GEOSGeometry*,double*);
    int (*GEOSHausdorffDistance_r)(GEOSContextHandle_t,const GEOSGeometry*,const GEOSGeometry*,double*);
    GEOSGeometry* (*GEOSBoundary_r)(GEOSContextHandle_t,const GEOSGeometry*);
    GEOSGeometry* (*GEOSConvexHull_r)(GEOSContextHandle_t,const GEOSGeometry*);
    GEOSGeometry* (*GEOSEnvelope_r)(GEOSContextHandle_t,const GEOSGeometry*);
    GEOSGeometry* (*GEOSGetCentroid_r)(GEOSContextHandle_t,const GEOSGeometry*);
    int (*GEOSGeomGetNumPoints_r)(GEOSContextHandle_t,const GEOSGeometry*);
    GEOSGeometry* (*GEOSGeomGetPointN_r)(GEOSContextHandle_t,const GEOSGeometry*,int);
    GEOSGeometry* (*GEOSGeomGetStartPoint_r)(GEOSContextHandle_t,const GEOSGeometry*);
    GEOSGeometry* (*GEOSGeomGetEndPoint_r)(GEOSContextHandle_t,const GEOSGeometry*);
    int (*GEOSGetNumInteriorRings_r)(GEOSContextHandle_t,const GEOSGeometry*);
    GEOSGeometry* (*GEOSGetInteriorRingN_r)(GEOSContextHandle_t,const GEOSGeometry*,int);
    GEOSGeometry* (*GEOSGetExteriorRing_r)(GEOSContextHandle_t,const GEOSGeometry*);
    int (*GEOSGetNumGeometries_r)(GEOSContextHandle_t,const GEOSGeometry*);
    GEOSGeometry* (*GEOSGetGeometryN_r)(GEOSContextHandle_t,const GEOSGeometry*,int);
    GEOSGeometry* (*GEOSDifference_r)(GEOSContextHandle_t,const GEOSGeometry*,const GEOSGeometry*);
    GEOSGeometry* (*GEOSSymDifference_r)(GEOSContextHandle_t,const GEOSGeometry*,const GEOSGeometry*);
    GEOSGeometry* (*GEOSIntersection_r)(GEOSContextHandle_t,const GEOSGeometry*,const GEOSGeometry*);
    GEOSGeometry* (*GEOSUnion_r)(GEOSContextHandle_t,const GEOSGeometry*,const GEOSGeometry*);
    GEOSGeometry* (*GEOSBuffer_r)(GEOSContextHandle_t,const GEOSGeometry*,double,int);
    char (*GEOSRelatePattern_r)(GEOSContextHandle_t,const GEOSGeometry*,const GEOSGeometry*,const char *);
    int (*GEOSGeomTypeId_r)(GEOSContextHandle_t,const GEOSGeometry*);
    int (*GEOSGetSRID_r)(GEOSContextHandle_t,const GEOSGeometry*);
    void (*GEOSSetSRID_r)(GEOSContextHandle_t,const GEOSGeometry*,int);

    GEOSGeometry* (*GEOSGeom_createPoint_r)(GEOSContextHandle_t,GEOSCoordSequence*);
    GEOSGeometry* (*GEOSGeom_createEmptyPoint_r)(GEOSContextHandle_t);
    GEOSGeometry* (*GEOSGeom_createLinearRing_r)(GEOSContextHandle_t,GEOSCoordSequence*);
    GEOSGeometry* (*GEOSGeom_createLineString_r)(GEOSContextHandle_t,GEOSCoordSequence*);
    GEOSGeometry* (*GEOSGeom_createEmptyLineString_r)(GEOSContextHandle_t);
    GEOSGeometry* (*GEOSGeom_createEmptyPolygon_r)(GEOSContextHandle_t);
    GEOSGeometry* (*GEOSGeom_createPolygon_r)(GEOSContextHandle_t,GEOSGeometry*,GEOSGeometry**,unsigned int);
    GEOSGeometry* (*GEOSGeom_createCollection_r)(GEOSContextHandle_t,int,GEOSGeometry**,unsigned int);
    GEOSGeometry* (*GEOSGeom_createEmptyCollection_r)(GEOSContextHandle_t,int);

    const GEOSCoordSequence* (*GEOSGeom_getCoordSeq_r)(GEOSContextHandle_t,const GEOSGeometry*);
    GEOSCoordSequence* (*GEOSCoordSeq_create_r)(GEOSContextHandle_t,unsigned int,unsigned int);
    int (*GEOSCoordSeq_getSize_r)(GEOSContextHandle_t,const GEOSCoordSequence*,unsigned int *);
    int (*GEOSCoordSeq_getX_r)(GEOSContextHandle_t,const GEOSCoordSequence*,unsigned int,double*);
    int (*GEOSCoordSeq_getY_r)(GEOSContextHandle_t,const GEOSCoordSequence*,unsigned int,double*);
    int (*GEOSCoordSeq_setX_r)(GEOSContextHandle_t,GEOSCoordSequence*,unsigned int,double);
    int (*GEOSCoordSeq_setY_r)(GEOSContextHandle_t,GEOSCoordSequence*,unsigned int,double);

    GEOSPreparedGeometry const *(*GEOSPrepare_r)(GEOSContextHandle_t,const GEOSGeometry*);
    void (*GEOSPreparedGeom_destroy_r)(GEOSContextHandle_t,const GEOSPreparedGeometry*);
    void (*GEOSGeom_destroy_r)(GEOSContextHandle_t,const GEOSGeometry*);
} geos_api;

typedef struct {
    GEOSContextHandle_t context;
    geos_api api;
    void *geos_lib;
} geos_handle_t;

#define GEOSisClosed_r(ctx,g) ctx->api.GEOSisClosed_r(ctx->context,g)
#define GEOSisEmpty_r(ctx,g) ctx->api.GEOSisEmpty_r(ctx->context,g)
#define GEOSisSimple_r(ctx,g) ctx->api.GEOSisSimple_r(ctx->context,g)
#define GEOSisRing_r(ctx,g) ctx->api.GEOSisRing_r(ctx->context,g)
#define GEOSisValid_r(ctx,g) ctx->api.GEOSisValid_r(ctx->context,g)
#define GEOSPreparedCovers_r(ctx,pg,g) ctx->api.GEOSPreparedCovers_r(ctx->context,pg,g)
#define GEOSPreparedCoveredBy_r(ctx,pg,g) ctx->api.GEOSPreparedCoveredBy_r(ctx->context,pg,g)
#define GEOSPreparedDisjoint_r(ctx,pg,g) ctx->api.GEOSPreparedDisjoint_r(ctx->context,pg,g)
#define GEOSPreparedIntersects_r(ctx,pg,g) ctx->api.GEOSPreparedIntersects_r(ctx->context,pg,g)
#define GEOSPreparedTouches_r(ctx,pg,g) ctx->api.GEOSPreparedTouches_r(ctx->context,pg,g)
#define GEOSPreparedCrosses_r(ctx,pg,g) ctx->api.GEOSPreparedCrosses_r(ctx->context,pg,g)
#define GEOSPreparedWithin_r(ctx,pg,g) ctx->api.GEOSPreparedWithin_r(ctx->context,pg,g)
#define GEOSPreparedContains_r(ctx,pg,g) ctx->api.GEOSPreparedContains_r(ctx->context,pg,g)
#define GEOSPreparedOverlaps_r(ctx,pg,g) ctx->api.GEOSPreparedOverlaps_r(ctx->context,pg,g)
#define GEOSEquals_r(ctx,g1,g2) ctx->api.GEOSEquals_r(ctx->context,g1,g2)
#define GEOSArea_r(ctx,g,d) ctx->api.GEOSArea_r(ctx->context,g,d)
#define GEOSLength_r(ctx,g,d) ctx->api.GEOSLength_r(ctx->context,g,d)
#define GEOSDistance_r(ctx,g1,g2,d) ctx->api.GEOSDistance_r(ctx->context,g1,g2,d)
#define GEOSHausdorffDistance_r(ctx,g1,g2,d) ctx->api.GEOSHausdorffDistance_r(ctx->context,g1,g2,d)
#define GEOSBoundary_r(ctx,g) ctx->api.GEOSBoundary_r(ctx->context,g)
#define GEOSConvexHull_r(ctx,g) ctx->api.GEOSConvexHull_r(ctx->context,g)
#define GEOSEnvelope_r(ctx,g) ctx->api.GEOSEnvelope_r(ctx->context,g)
#define GEOSGetCentroid_r(ctx,g) ctx->api.GEOSGetCentroid_r(ctx->context,g)
#define GEOSGeomGetNumPoints_r(ctx,g) ctx->api.GEOSGeomGetNumPoints_r(ctx->context,g)
#define GEOSGeomGetPointN_r(ctx,g,i) ctx->api.GEOSGeomGetPointN_r(ctx->context,g,i)
#define GEOSGeomGetStartPoint_r(ctx,g) ctx->api.GEOSGeomGetStartPoint_r(ctx->context,g)
#define GEOSGeomGetEndPoint_r(ctx,g) ctx->api.GEOSGeomGetEndPoint_r(ctx->context,g)
#define GEOSGetNumInteriorRings_r(ctx,g) ctx->api.GEOSGetNumInteriorRings_r(ctx->context,g)
#define GEOSGetInteriorRingN_r(ctx,g,i) ctx->api.GEOSGetInteriorRingN_r(ctx->context,g,i)
#define GEOSGetExteriorRing_r(ctx,g) ctx->api.GEOSGetExteriorRing_r(ctx->context,g)
#define GEOSGetNumGeometries_r(ctx,g) ctx->api.GEOSGetNumGeometries_r(ctx->context,g)
#define GEOSGetGeometryN_r(ctx,g,i) ctx->api.GEOSGetGeometryN_r(ctx->context,g,i)
#define GEOSDifference_r(ctx,g1,g2) ctx->api.GEOSDifference_r(ctx->context,g1,g2)
#define GEOSSymDifference_r(ctx,g1,g2) ctx->api.GEOSSymDifference_r(ctx->context,g1,g2)
#define GEOSIntersection_r(ctx,g1,g2) ctx->api.GEOSIntersection_r(ctx->context,g1,g2)
#define GEOSUnion_r(ctx,g1,g2) ctx->api.GEOSUnion_r(ctx->context,g1,g2)
#define GEOSBuffer_r(ctx,g,d,i) ctx->api.GEOSBuffer_r(ctx->context,g,d,i)
#define GEOSRelatePattern_r(ctx,g1,g2,c) ctx->api.GEOSRelatePattern_r(ctx->context,g1,g2,c)
#define GEOSGeomTypeId_r(ctx,g) ctx->api.GEOSGeomTypeId_r(ctx->context,g)
#define GEOSGetSRID_r(ctx,g) ctx->api.GEOSGetSRID_r(ctx->context,g)
#define GEOSSetSRID_r(ctx,g,i) ctx->api.GEOSSetSRID_r(ctx->context,g,i)
#define GEOSGeom_createPoint_r(ctx,cs) ctx->api.GEOSGeom_createPoint_r(ctx->context,cs)
#define GEOSGeom_createEmptyPoint_r(ctx) ctx->api.GEOSGeom_createEmptyPoint_r(ctx->context)
#define GEOSGeom_createLinearRing_r(ctx,cs) ctx->api.GEOSGeom_createLinearRing_r(ctx->context,cs)
#define GEOSGeom_createLineString_r(ctx,cs) ctx->api.GEOSGeom_createLineString_r(ctx->context,cs)
#define GEOSGeom_createEmptyLineString_r(ctx) ctx->api.GEOSGeom_createEmptyLineString_r(ctx->context)
#define GEOSGeom_createEmptyPolygon_r(ctx) ctx->api.GEOSGeom_createEmptyPolygon_r(ctx->context)
#define GEOSGeom_createPolygon_r(ctx,g1,g2,i) ctx->api.GEOSGeom_createPolygon_r(ctx->context,g1,g2,i)
#define GEOSGeom_createCollection_r(ctx,i1,g,i2) ctx->api.GEOSGeom_createCollection_r(ctx->context,i1,g,i2)
#define GEOSGeom_createEmptyCollection_r(ctx,i) ctx->api.GEOSGeom_createEmptyCollection_r(ctx->context,i)
#define GEOSGeom_getCoordSeq_r(ctx,g) ctx->api.GEOSGeom_getCoordSeq_r(ctx->context,g)
#define GEOSCoordSeq_create_r(ctx,i1,i2) ctx->api.GEOSCoordSeq_create_r(ctx->context,i1,i2)
#define GEOSCoordSeq_getSize_r(ctx,cs,i) ctx->api.GEOSCoordSeq_getSize_r(ctx->context,cs,i)
#define GEOSCoordSeq_getX_r(ctx,cs,i,d) ctx->api.GEOSCoordSeq_getX_r(ctx->context,cs,i,d)
#define GEOSCoordSeq_getY_r(ctx,cs,i,d) ctx->api.GEOSCoordSeq_getY_r(ctx->context,cs,i,d)
#define GEOSCoordSeq_setX_r(ctx,cs,i,d) ctx->api.GEOSCoordSeq_setX_r(ctx->context,cs,i,d)
#define GEOSCoordSeq_setY_r(ctx,cs,i,d) ctx->api.GEOSCoordSeq_setY_r(ctx->context,cs,i,d)
#define GEOSPrepare_r(ctx,g) ctx->api.GEOSPrepare_r(ctx->context,g)
#define GEOSPreparedGeom_destroy_r(ctx,pg) ctx->api.GEOSPreparedGeom_destroy_r(ctx->context,pg)
#define GEOSGeom_destroy_r(ctx,g) ctx->api.GEOSGeom_destroy_r(ctx->context,g)
#define GEOSversion(ctx) ctx->api.GEOSversion()

#endif

#endif
