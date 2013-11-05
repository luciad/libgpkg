/*
 * Copyright 2013 Luciad (http://www.luciad.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef GPKG_H
#define GPKG_H

#include <sqlite3ext.h>

#ifdef GPKG_HAVE_CONFIG_H
#include "config.h"
#endif

/**
 * \addtogroup gpkg Library initialization and metadata
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef GPKG_EXPORT
#define GPKG_EXPORT
#endif

#ifndef GPKG_CALL
#define GPKG_CALL
#endif

/**
 * Returns the version number of libgpkg as a string.
 * @return a version number
 */
GPKG_EXPORT const char *GPKG_CALL gpkg_libversion();

/**
 * Entry point for the libgpkg SQLite extension that forces usage of the GeoPackage database schema.
 */
GPKG_EXPORT int GPKG_CALL sqlite3_gpkg_init(sqlite3 *db, const char **pzErrMsg, const sqlite3_api_routines *pThunk);

/**
 * Entry point for the libgpkg SQLite extension that attempts to autodetect the schema to use.
 */
GPKG_EXPORT int GPKG_CALL sqlite3_gpkg_auto_init(sqlite3 *db, const char **pzErrMsg, const sqlite3_api_routines *pThunk);

/**
 * Entry point for the libgpkg SQLite extension that forces usage of the Spatialite 2.x database schema.
 */
GPKG_EXPORT int GPKG_CALL sqlite3_gpkg_spl2_init(sqlite3 *db, const char **pzErrMsg, const sqlite3_api_routines *pThunk);

/**
 * Entry point for the libgpkg SQLite extension that forces usage of the Spatialite 3.x database schema.
 */
GPKG_EXPORT int GPKG_CALL sqlite3_gpkg_spl3_init(sqlite3 *db, const char **pzErrMsg, const sqlite3_api_routines *pThunk);

/**
 * Entry point for the libgpkg SQLite extension that forces usage of the Spatialite 4.x database schema.
 */
GPKG_EXPORT int GPKG_CALL sqlite3_gpkg_spl4_init(sqlite3 *db, const char **pzErrMsg, const sqlite3_api_routines *pThunk);

#ifdef __cplusplus
}
#endif

/** @} */

#endif
