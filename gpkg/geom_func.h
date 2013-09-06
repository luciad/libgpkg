#ifndef GPKG_GEOM_FUNC_H
#define GPKG_GEOM_FUNC_H

#include "error.h"
#include "spatialdb.h"

void geom_func_init(sqlite3*db, const struct spatialdb *spatialDb, error_t *error);

#endif
