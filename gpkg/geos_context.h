#ifndef GPKG_GEOS_CONTEXT_H
#define GPKG_GEOS_CONTEXT_H

#include "geos.h"
#include "error.h"

void geom_geos_clear_error();

void geom_geos_get_error(errorstream_t *error);

#if GPKG_GEOM_FUNC == GPKG_GEOS
geos_handle_t * geom_geos_init(errorstream_t *error);
#else
geos_handle_t *geom_geos_init(char const *geos_lib, errorstream_t *error);
#endif

void geom_geos_destroy(geos_handle_t * geos);

#endif
