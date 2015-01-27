#include <stdarg.h>
#include <stdio.h>
#include "geos_context.h"
#include "sqlite.h"
#include "tls.h"

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

void geom_geos_print_error() {
  char *err = GPKG_TLS_GET(last_geos_error);
  if (err != NULL) {
    printf("%s\n", err);
    sqlite3_free(err);
    GPKG_TLS_SET(last_geos_error, NULL);
  }
}

static void geom_null_msg_handler(const char *fmt, ...) {
}

static void geom_tls_msg_handler(const char *fmt, ...) {
  geom_geos_clear_error();

  int result;
  va_list args;
  va_start(args, fmt);
  char *err = sqlite3_vmprintf(fmt, args);
  GPKG_TLS_SET(last_geos_error, err);
  va_end(args);
}

GEOSContextHandle_t geom_geos_init() {
  GPKG_TLS_KEY_CREATE(last_geos_error);
  return initGEOS_r(geom_null_msg_handler, geom_tls_msg_handler);
}

void geom_geos_destroy(GEOSContextHandle_t geos) {
  finishGEOS_r(geos);
}
