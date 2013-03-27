#ifndef GPKG_H
#define GPKG_H

#include <sqlite3.h>

const char *gpkg_libversion(void);
int gpkg_extension_init(sqlite3*, const char **, const void *);

#endif
