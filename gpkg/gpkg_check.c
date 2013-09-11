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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "check.h"
#include "sql.h"

typedef struct {
  int    found;
  char    *name;
} Column;

static int gpkg_contents_table_exists(sqlite3 *db, const char *db_name, const table_info_t *const *tables, error_t *error) {
  sqlite3_stmt *stmt = NULL;
  int result = sql_init_stmt(&stmt, db, "SELECT table_name FROM gpkg_contents");
  int exists = 0;

  if (result != SQLITE_OK) {
    return result;
  }
  int stmt_res = sqlite3_step(stmt);
  if (stmt_res == SQLITE_DONE) {
    return result;
  }
  const char *table_name;
  while (stmt_res == SQLITE_ROW) {
    table_name = (const char *)sqlite3_column_text(stmt, 0);
    result = sql_check_table_exists(db, db_name, table_name, &exists);
    if (result != SQLITE_OK) {
      return result;
    }

    if (!exists) {
      error_append(error, "Table %s.%s does not exist", db_name, table_name);
      return SQLITE_OK;
    }
    stmt_res = sqlite3_step(stmt);
  }
  return result;
}

static int gpkg_contents_check_spatialref_srsid_row(sqlite3 *db, sqlite3_stmt *stmt, void *data) {
  error_append((error_t *)data, "Missing spatial reference with id=%s in gpkg_contents for table=%s", ((const char *)sqlite3_column_text(stmt, 1)), ((const char *)sqlite3_column_text(stmt, 0)));
  return SQLITE_OK;
}

static int gpkg_contents_check_spatialref_srsid(sqlite3 *db, const char *db_name, const table_info_t *const *tables, error_t *error) {
  return sql_exec_stmt(
           db, gpkg_contents_check_spatialref_srsid_row, NULL, error,
           "SELECT table_name, srs_id FROM gpkg_contents WHERE NOT EXISTS (SELECT 1 FROM gpkg_spatial_ref_sys WHERE gpkg_contents.srs_id = gpkg_spatial_ref_sys.srs_id)"
         );
}


static int gpkg_geometry_columns_check_spatialref_srsid_row(sqlite3 *db, sqlite3_stmt *stmt, void *data) {
  error_append((error_t *)data, "Missing spatial reference with id=%s in geometry_columns for table=%s", ((const char *)sqlite3_column_text(stmt, 1)), ((const char *)sqlite3_column_text(stmt, 0)));
  return SQLITE_OK;
}

static int gpkg_geometry_columns_check_spatialref_srsid(sqlite3 *db, const char *db_name, const table_info_t *const *tables, error_t *error) {
  return sql_exec_stmt(
           db, gpkg_geometry_columns_check_spatialref_srsid_row, NULL, error,
           "SELECT table_name, srs_id FROM gpkg_geometry_columns WHERE NOT EXISTS (SELECT 1 FROM gpkg_spatial_ref_sys WHERE gpkg_geometry_columns.srs_id = gpkg_spatial_ref_sys.srs_id)"
         );
}

static int gpkg_contents_geometry_table_check_row(sqlite3 *db, sqlite3_stmt *stmt, void *data) {
  error_append((error_t *)data, "Table=%s not present in gpkg_contents or gpkg_geometry columns", ((const char *)sqlite3_column_text(stmt, 0)));
  return SQLITE_OK;
}

static int gpkg_contents_geometry_table_check(sqlite3 *db, const char *db_name, const table_info_t *const *tables, error_t *error) {
  int result = sql_exec_stmt(
                 db, gpkg_contents_geometry_table_check_row, NULL, error,
                 "SELECT table_name FROM gpkg_contents WHERE data_type='features' AND table_name NOT IN (SELECT table_name FROM gpkg_geometry_columns)"
               );

  if (result != SQLITE_OK) {
    return result;
  }
  return sql_exec_stmt(
           db, gpkg_contents_geometry_table_check_row, NULL, error,
           "SELECT table_name FROM gpkg_geometry_columns WHERE table_name NOT IN (SELECT table_name FROM gpkg_contents WHERE data_type='features')"
         );
}

static int gpkg_contents_tilemetadata_table_check_row(sqlite3 *db, sqlite3_stmt *stmt, void *data) {
  error_append((error_t *)data, "Table=%s not present in gpkg_contents or gpkg_tile_matrix_metadata", ((const char *)sqlite3_column_text(stmt, 0)));
  return SQLITE_OK;
}

static int gpkg_contents_tilemetadata_table_check(sqlite3 *db, const char *db_name, const table_info_t *const *tables, error_t *error) {
  int result = sql_exec_stmt(
                 db, gpkg_contents_tilemetadata_table_check_row, NULL, error,
                 "SELECT table_name FROM gpkg_contents WHERE data_type='tiles' AND table_name NOT IN (SELECT table_name FROM gpkg_tile_matrix_metadata)"
               );

  if (result != SQLITE_OK) {
    return result;
  }
  return sql_exec_stmt(
           db, gpkg_contents_tilemetadata_table_check_row, NULL, error,
           "SELECT table_name FROM gpkg_tile_matrix_metadata WHERE table_name NOT IN (SELECT table_name FROM gpkg_contents WHERE data_type='tiles')"
         );
}


static int gpkg_find_column_table(sqlite3 *db, sqlite3_stmt *stmt, void *data) {
  Column *c = (Column *)data;
  if (sqlite3_strnicmp(c->name, ((const char *)sqlite3_column_text(stmt, 1)), strlen(c->name)+1) == 0) {
    c->found = 1;
  }
  return SQLITE_OK;
}


static int gpkg_geometry_columns_table_column_check_row(sqlite3 *db, sqlite3_stmt *stmt, void *data) {
  Column *c = malloc(sizeof(Column));
  c->found = 0;
  (*c).name = ((char *)sqlite3_column_text(stmt, 1));
  int result = sql_exec_stmt(
                 db, gpkg_find_column_table, NULL, c ,
                 "PRAGMA table_info(%w)", ((const char *)sqlite3_column_text(stmt, 0)));
  if (!c->found) {
    error_append((error_t *)data, "The column %s is not found in table %s", c->name, ((const char *)sqlite3_column_text(stmt, 0)));
  }
  free(c);
  return SQLITE_OK;
}

static int gpkg_geometry_columns_table_column_check(sqlite3 *db, const char *db_name, const table_info_t *const *tables, error_t *error) {
  int result = sql_exec_stmt(
                 db, gpkg_geometry_columns_table_column_check_row, NULL, error,
                 "SELECT table_name, column_name FROM gpkg_geometry_columns"
               );

  return result;
}


typedef int(*check_func)(sqlite3 *db, const char *db_name, const table_info_t *const *tables, error_t *error);

static check_func checks[] = {
  gpkg_contents_table_exists,
  gpkg_contents_check_spatialref_srsid,
  gpkg_geometry_columns_check_spatialref_srsid,
  gpkg_contents_geometry_table_check,
  gpkg_contents_tilemetadata_table_check,
  gpkg_geometry_columns_table_column_check,
  NULL
};


int gpkg_check_database(sqlite3 *db, const char *db_name, const table_info_t *const *tables, error_t *error) {
  int result = SQLITE_OK;

  check_func *current_func = checks;
  while (*current_func != NULL) {
    result = (*current_func)(db, db_name, tables, error);
    if (result != SQLITE_OK) {
      break;
    }
    current_func++;
  }

  return result;
}
