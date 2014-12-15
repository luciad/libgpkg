LOCAL_PATH:= $(call my-dir)

include ${CLEAR_VARS}

LOCAL_SRC_FILES := \
    gpkg/binstream.c \
    gpkg/blobio.c \
    gpkg/error.c \
    gpkg/fp.c \
    gpkg/geomio.c \
    gpkg/gpkg.c \
    gpkg/gpkg_db.c \
    gpkg/gpkg_geom.c \
    gpkg/i18n.c \
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

include $(LOCAL_PATH)/version.txt

LOCAL_CFLAGS := \
    -fvisibility=hidden \
    -std=c99 \
    -DLIBGPKG_VERSION="\"$(gpkg_VERSION_MAJOR).$(gpkg_VERSION_MINOR).$(gpkg_VERSION_PATCH)\"" \
    -DGPKG_EXPORT="__attribute__((visibility(\"default\")))"

include $(BUILD_SHARED_LIBRARY)

# Build the shell for android
# To link the shared lib, add its path to LD_LIBRARY_PATH
# Currently segfaults when trying to run. No idea why yet.
#
#include $(CLEAR_VARS)
#
#LOCAL_SRC_FILES := shell/shell.c
#
#LOCAL_C_INCLUDES := \
#    $(LOCAL_PATH)/sqlite \
#    $(LOCAL_PATH)/gpkg
#
#LOCAL_MODULE := gpkg_shell
#
#LOCAL_STATIC_LIBRARIES := sqlite
#LOCAL_SHARED_LIBRARIES := gpkg
#
#LOCAL_CFLAGS += -fPIE
#LOCAL_LDFLAGS += -fPIE -pie
#
#include $(BUILD_EXECUTABLE)
