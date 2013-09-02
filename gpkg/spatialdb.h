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
#ifndef GPKG_SPATIALDB_H
#define GPKG_SPATIALDB_H

#include "error.h"
#include "blobio.h"
#include "sqlite.h"
#include "gpkg.h"

typedef enum {
  GEOPACKAGE,
  SPATIALITE3,
  SPATIALITE4
} spatialdb_schema;

typedef struct {
  const char *name;
  int(*init)(sqlite3 *db, const char *db_name, error_t *error);
  int(*check)(sqlite3 *db, const char *db_name, error_t *error);
  int(*write_blob_header)(binstream_t *stream, geom_blob_header_t *header, error_t *error);
  int(*read_blob_header)(binstream_t *stream, geom_blob_header_t *header, error_t *error);
  int(*writer_init)(geom_blob_writer_t *writer);
  int(*writer_init_srid)(geom_blob_writer_t *writer, int32_t srid);
  void(*writer_destroy)(geom_blob_writer_t *writer);
  int(*add_geometry_column)(sqlite3 *db, const char *db_name, const char *table_name, const char *column_name, const char *geom_type, int srs_id, int z, int m, error_t *error);
  int(*create_tiles_table)(sqlite3 *db, const char *db_name, const char *table_name, error_t *error);
  int(*create_spatial_index)(sqlite3 *db, const char *db_name, const char *table_name, const char *column_name, error_t *error);
  int(*fill_envelope)(binstream_t *stream, geom_envelope_t *envelope, error_t *error);
  int(*read_geometry_header)(binstream_t *stream, geom_header_t *header, error_t *error);
  int(*read_geometry)(binstream_t *stream, geom_consumer_t const *consumer, error_t *error);
} spatialdb_t;

int spatialdb_init(sqlite3 *db, const char **pzErrMsg, const sqlite3_api_routines *pThunk, spatialdb_schema schema);

#endif