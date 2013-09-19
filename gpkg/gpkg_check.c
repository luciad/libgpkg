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

/*
 * Check that each 'features' table in gpkg_contents has at least one corresponding row in gpkg_geometry_columns.
 * This is not covered by a foreign key reference.
 */
static int gpkg_contents_geometry_table_check_row(sqlite3 *db, sqlite3_stmt *stmt, void *data) {
  error_append(
    (error_t *)data,
    "gpkg_contents: table '%s' has data_type 'features' but no rows exist in gpkg_geometry_columns for this table",
    sqlite3_column_text(stmt, 0)
  );
  return SQLITE_OK;
}

static int gpkg_contents_geometry_table_check(sqlite3 *db, const char *db_name, error_t *error) {
  return sql_exec_stmt(
           db, gpkg_contents_geometry_table_check_row, NULL, error,
           "SELECT table_name FROM \"%w\".gpkg_contents WHERE data_type='features' AND table_name NOT IN (SELECT table_name FROM \"%w\".gpkg_geometry_columns)",
           db_name, db_name
         );
}

/*
 * Check that each 'tiles' table in gpkg_contents has at least one corresponding row in gpkg_tile_matrix_metadata.
 * This is not covered by a foreign key reference.
 */
static int gpkg_contents_tilemetadata_table_check_row(sqlite3 *db, sqlite3_stmt *stmt, void *data) {
  error_append(
    (error_t *)data,
    "gpkg_contents: table '%s' has data_type 'tiles' but no rows exist in gpkg_tile_matrix_metadata for this table",
    sqlite3_column_text(stmt, 0)
  );
  return SQLITE_OK;
}

static int gpkg_contents_tilemetadata_table_check(sqlite3 *db, const char *db_name, error_t *error) {
  return sql_exec_stmt(
           db, gpkg_contents_tilemetadata_table_check_row, NULL, error,
           "SELECT table_name FROM \"%w\".gpkg_contents WHERE data_type='tiles' AND table_name NOT IN (SELECT table_name FROM \"%w\".gpkg_tile_matrix_metadata)",
           db_name, db_name
         );
}

/*
 * Check that each meta tables that reference columns all refer to tables and columns that actually exist.
 */
typedef struct {
  const char *db_name;
  const char *source_table;
  error_t *error;
} column_check_data_t;

static int gpkg_table_column_check_row(sqlite3 *db, sqlite3_stmt *stmt, void *data) {
  int result = SQLITE_OK;
  int exists = 0;
  column_check_data_t *c = (column_check_data_t *)data;
  char *table_name = sqlite3_mprintf("%s", sqlite3_column_text(stmt, 0));
  char *column_name;
  if (sqlite3_column_type(stmt, 1) == SQLITE_NULL) {
    column_name = NULL;
  } else {
    column_name = sqlite3_mprintf("%s", sqlite3_column_text(stmt, 1));
    if (column_name == NULL) {
      result = SQLITE_NOMEM;
      goto exit;
    }
  }


  result = sql_check_table_exists(db, c->db_name, table_name, &exists);
  if (result == SQLITE_OK && !exists) {
    error_append(c->error, "%s: table '%s' does not exist", c->source_table, table_name);
  }

  if (exists && column_name != NULL) {
    exists = 0;
    result = sql_check_column_exists(db, c->db_name, table_name, column_name, &exists);
    if (result == SQLITE_OK && !exists) {
      error_append(c->error, "%s: column '%s.%s' does not exist", c->source_table, table_name, column_name);
    }
  }

exit:
  sqlite3_free(table_name);
  sqlite3_free(column_name);
  return result;
}

static int gpkg_table_column_check(sqlite3 *db, const char *db_name, const char *table_name, const char *table_column, const char *column_column, error_t *error) {
  column_check_data_t c;
  c.db_name = db_name;
  c.source_table = table_name;
  c.error = error;

  if (column_column != NULL) {
    return sql_exec_stmt(db, gpkg_table_column_check_row, NULL, &c, "SELECT \"%w\", \"%w\" FROM \"%w\".\"%w\"", table_column, column_column, db_name, table_name);
  } else {
    return sql_exec_stmt(db, gpkg_table_column_check_row, NULL, &c, "SELECT \"%w\", NULL FROM \"%w\".\"%w\"", table_column, db_name, table_name);
  }
}

static int gpkg_contents_columns_table_column_check(sqlite3 *db, const char *db_name, error_t *error) {
  return gpkg_table_column_check(db, db_name, "gpkg_contents", "table_name", NULL, error);
}

static int gpkg_extensions_table_column_check(sqlite3 *db, const char *db_name, error_t *error) {
  return gpkg_table_column_check(db, db_name, "gpkg_extensions", "table_name", "column_name", error);
}

static int gpkg_geometry_columns_table_column_check(sqlite3 *db, const char *db_name, error_t *error) {
  return gpkg_table_column_check(db, db_name, "gpkg_geometry_columns", "table_name", "column_name", error);
}

static int gpkg_tile_matrix_metadata_table_column_check(sqlite3 *db, const char *db_name, error_t *error) {
  return gpkg_table_column_check(db, db_name, "gpkg_tile_matrix_metadata", "table_name", NULL, error);
}

static int gpkg_data_columns_table_column_check(sqlite3 *db, const char *db_name, error_t *error) {
  return gpkg_table_column_check(db, db_name, "gpkg_data_columns", "table_name", "column_name", error);
}

static int gpkg_metadata_reference_table_column_check(sqlite3 *db, const char *db_name, error_t *error) {
  return gpkg_table_column_check(db, db_name, "gpkg_metadata_reference", "table_name", "column_name", error);
}

typedef int(*check_func)(sqlite3 *db, const char *db_name, error_t *error);

static check_func checks[] = {
  gpkg_contents_columns_table_column_check,
  gpkg_extensions_table_column_check,
  gpkg_geometry_columns_table_column_check,
  gpkg_tile_matrix_metadata_table_column_check,
  gpkg_data_columns_table_column_check,
  gpkg_metadata_reference_table_column_check,
  gpkg_contents_geometry_table_check,
  gpkg_contents_tilemetadata_table_check,
  NULL
};


int gpkg_check_database(sqlite3 *db, const char *db_name, error_t *error) {
  int result = SQLITE_OK;

  check_func *current_func = checks;
  while (*current_func != NULL) {
    result = (*current_func)(db, db_name, error);
    if (result != SQLITE_OK) {
      break;
    }
    current_func++;
  }

  return result;
}
