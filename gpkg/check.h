#ifndef GPKG_CHECK_H
#define GPKG_CHECK_H

#include "sqlite.h"
#include "error.h"

int check_gpkg(sqlite3 *db, char *db_name, error_t *error);

#endif