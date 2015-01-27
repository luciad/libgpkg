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

/**
 * Abstraction layer for spatial databases.
 */
typedef struct spatialdb {
  /**
   * The name of the spatial database type.
   */
  const char *name;
  /**
   * Initializes a database. Implementations of this function should perform any required setup work like registering
   * functions.
   */
  void(*init)(sqlite3 *db, const struct spatialdb *spatialDb, errorstream_t *error);
  /**
   * Initializes the metadata tables for this spatial database type.
   */
  int(*init_meta)(sqlite3 *db, const char *db_name, errorstream_t *error);
  /**
   * Verifies the metadata tables for this spatial database type reporting any errors to the given error object.
   */
  int(*check_meta)(sqlite3 *db, const char *db_name, int check_flags,  errorstream_t *error);
  /**
   * Writes the spatial database specific blob header to the given stream. When this function exits the stream
   * will be positioned directly after the header.
   */
  int(*write_blob_header)(binstream_t *stream, geom_blob_header_t *header, errorstream_t *error);
  /**
   * Reads the spatial database specific blob header from the given stream. When this function exits the stream
   * will be positioned directly after the header.
   */
  int(*read_blob_header)(binstream_t *stream, geom_blob_header_t *header, errorstream_t *error);
  /*
   * Initializes a spatial database specific geometry blob writer. If applicable, an implementation dependent default
   * SRID will be used.
   */
  int(*writer_init)(geom_blob_writer_t *writer);
  /*
   * Initializes a spatial database specific geometry blob writer.
   */
  int(*writer_init_srid)(geom_blob_writer_t *writer, int32_t srid);
  /**
   * Destroys a geometry blob writer.
   */
  void(*writer_destroy)(geom_blob_writer_t *writer, int free_data);
  /**
   * Adds a geometry column to an existing table.
   */
  int(*add_geometry_column)(sqlite3 *db, const char *db_name, const char *table_name, const char *column_name, const char *geom_type, int srs_id, int z, int m, errorstream_t *error);
  /**
   * Creates a tile pyramid table.
   */
  int(*create_tiles_table)(sqlite3 *db, const char *db_name, const char *table_name, errorstream_t *error);
  /**
   * Creates a spatial index on a given table column.
   */
  int(*create_spatial_index)(sqlite3 *db, const char *db_name, const char *table_name, const char *geometry_column_name, const char *id_column_name, errorstream_t *error);
  /**
   * Populates a geometry envelope based on a geometry blob. The stream is expected to be positioned at the start
   * of the geometry body (i.e., immediately after the blob header). When this function returns the stream is positioned
   * immediately after the geometry body.
   */
  int(*fill_envelope)(binstream_t *stream, geom_envelope_t *envelope, errorstream_t *error);
  /**
   * Populates a geometry header based on a geometry blob. The stream is expected to be positioned at the start
   * of the geometry body (i.e., immediately after the blob header). When this function returns the stream is positioned
   * immediately after the geometry header.
   */
  int(*read_geometry_header)(binstream_t *stream, geom_header_t *header, errorstream_t *error);
  /**
   * Reads a geometry from the given stream. The stream is expected to be positioned at the start
   * of the geometry body (i.e., immediately after the blob header). When this function returns the stream is positioned
   * immediately after the geometry body.
   */
  int(*read_geometry)(binstream_t *stream, geom_consumer_t const *consumer, errorstream_t *error);
} spatialdb_t;

/**
 * Returns the GeoPackage spatial database schema.
 */
const spatialdb_t *spatialdb_geopackage_schema();

/**
 * Returns the Spatialite 2.x spatial database schema.
 */
const spatialdb_t *spatialdb_spatialite2_schema();

/**
 * Returns the Spatialite 3.x spatial database schema.
 */
const spatialdb_t *spatialdb_spatialite3_schema();

/**
 * Returns the Spatialite 4.x spatial database schema.
 */
const spatialdb_t *spatialdb_spatialite4_schema();

/**
 * Initializes the given sqlite database with a specific spatial database schema. If the schema is set to NULL,
 * this function will attempt to autodetect the applicable schema.
 */
int spatialdb_init(sqlite3 *db, const char **pzErrMsg, const sqlite3_api_routines *pThunk, const spatialdb_t *schema);

#endif
