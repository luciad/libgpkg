LOCAL_PATH:= $(call my-dir)

include ${CLEAR_VARS}

LOCAL_SRC_FILES := \
    gpkg/binstream.c \
    gpkg/blobio.c \
    gpkg/check.c \
    gpkg/error.c \
    gpkg/fp.c \
    gpkg/geomio.c \
    gpkg/gpkg.c \
    gpkg/gpkg_db.c \
    gpkg/gpkg_geom.c \
    gpkg/spatialdb.c \
    gpkg/spl_db.c \
    gpkg/spl_geom.c \
    gpkg/sql.c \
    gpkg/strbuf.c \
    gpkg/wkb.c \
    gpkg/wkt.c \

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/sqlite

LOCAL_MODULE := gpkg

LOCAL_CFLAGS := \
    -fvisibility=hidden \
    -std=c99 \
    -DLIBGPKG_VERSION="\"0.8.0\"" \
    -DGPKG_EXPORT="__attribute__((visibility(\"default\")))"

include $(BUILD_SHARED_LIBRARY)