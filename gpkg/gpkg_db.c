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
#include "spatialdb_internal.h"
#include "gpkg_geom.h"
#include "sql.h"
#include "sqlite.h"

#define N NULL_VALUE
#define D(v) DOUBLE_VALUE(v)
#define I(v) INT_VALUE(v)
#define T(v) TEXT_VALUE(v)
#define F(v) FUNC_VALUE(v)

static column_info_t gpkg_spatial_ref_sys_columns[] = {
  {"srs_name", "TEXT", N, SQL_NOT_NULL, NULL},
  {"srs_id", "INTEGER", N, SQL_NOT_NULL | SQL_PRIMARY_KEY, NULL},
  {"organization", "TEXT", N, SQL_NOT_NULL, NULL},
  {"organization_coordsys_id", "INTEGER", N, SQL_NOT_NULL, NULL},
  {"definition", "TEXT", N, SQL_NOT_NULL, NULL},
  {"description", "TEXT", N, 0, NULL},
  {NULL, NULL, N, 0, NULL}
};
static value_t gpkg_spatial_ref_sys_data[] = {
  T("Undefined Cartesian"), I(-1), T("NONE"), I(-1), T("undefined"), N,
  T("Undefined Geographic"), I(0), T("NONE"), I(0), T("undefined"), N,
  T("WGS 84"), I(4326), T("EPSG"), I(4326), T("GEOGCS[\"WGS 84\", DATUM[\"WGS_1984\", SPHEROID[\"WGS 84\",6378137,298.257223563, AUTHORITY[\"EPSG\",\"7030\"]], AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\" 8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]], AUTHORITY[\"EPSG\",\"4326\"]]"), N
};
static table_info_t gpkg_spatial_ref_sys = {
  "gpkg_spatial_ref_sys",
  gpkg_spatial_ref_sys_columns,
  gpkg_spatial_ref_sys_data, 3
};

static column_info_t gpkg_contents_columns[] = {
  {"table_name", "TEXT", N, SQL_NOT_NULL | SQL_PRIMARY_KEY, NULL},
  {"data_type", "TEXT", N, SQL_NOT_NULL, NULL},
  {"identifier", "TEXT", N, SQL_UNIQUE(1), NULL},
  {"description", "TEXT", T(""), 0, NULL},
  {"last_change", "DATETIME", F("strftime('%%Y-%%m-%%dT%%H:%%M:%%fZ',CURRENT_TIMESTAMP)"), SQL_NOT_NULL, NULL},
  {"min_x", "DOUBLE", N, 0, NULL},
  {"min_y", "DOUBLE", N, 0, NULL},
  {"max_x", "DOUBLE", N, 0, NULL},
  {"max_y", "DOUBLE", N, 0, NULL},
  {"srs_id", "INTEGER", N, 0, "CONSTRAINT fk_srid__gpkg_spatial_ref_sys_srs_id REFERENCES gpkg_spatial_ref_sys(srs_id)"},
  {NULL, NULL, N, 0, NULL}
};
static table_info_t gpkg_contents = {
  "gpkg_contents",
  gpkg_contents_columns,
  NULL, 0
};

static column_info_t gpkg_extensions_columns[] = {
  {"table_name", "TEXT", N, SQL_UNIQUE(1), NULL},
  {"column_name", "TEXT", N, SQL_UNIQUE(1), NULL},
  {"extension_name", "TEXT", N, SQL_NOT_NULL | SQL_UNIQUE(1), NULL},
  {"definition", "TEXT", N, SQL_NOT_NULL, NULL},
  {"scope", "TEXT", N, SQL_NOT_NULL, NULL},
  {NULL, NULL, N, 0, NULL}
};
static table_info_t gpkg_extensions = {
  "gpkg_extensions",
  gpkg_extensions_columns,
  NULL, 0
};

static column_info_t gpkg_geometry_columns_columns[] = {
  {"table_name", "TEXT", N, SQL_NOT_NULL | SQL_PRIMARY_KEY | SQL_UNIQUE(1),  "CONSTRAINT fk_table_name__gpkg_contents_table_name REFERENCES gpkg_contents(table_name)"},
  {"column_name", "TEXT", N, SQL_NOT_NULL | SQL_PRIMARY_KEY, NULL},
  {"geometry_type_name", "TEXT", N, SQL_NOT_NULL, NULL},
  {"srs_id", "INTEGER", N, SQL_NOT_NULL, "CONSTRAINT fk_srs_id__gpkg_spatial_ref_sys_srs_id REFERENCES gpkg_spatial_ref_sys(srs_id)"},
  {"z", "TINYINT", N, SQL_NOT_NULL, NULL},
  {"m", "TINYINT", N, SQL_NOT_NULL, NULL},
  {NULL, NULL, N, 0, NULL}
};
static table_info_t gpkg_geometry_columns = {
  "gpkg_geometry_columns",
  gpkg_geometry_columns_columns,
  NULL, 0
};

static column_info_t gpkg_tile_matrix_set_columns[] = {
  {"table_name", "TEXT", N, SQL_NOT_NULL | SQL_PRIMARY_KEY, "CONSTRAINT fk_table_name__gpkg_contents_table_name REFERENCES gpkg_contents(table_name)"},
  {"srs_id", "INTEGER", N, SQL_NOT_NULL, "CONSTRAINT fk_srs_id__gpkg_spatial_ref_sys_srs_id REFERENCES gpkg_spatial_ref_sys(srs_id)"},
  {"min_x", "DOUBLE", N, SQL_NOT_NULL, NULL},
  {"min_y", "DOUBLE", N, SQL_NOT_NULL, NULL},
  {"max_x", "DOUBLE", N, SQL_NOT_NULL, NULL},
  {"max_y", "DOUBLE", N, SQL_NOT_NULL, NULL},
  {NULL, NULL, N, 0, NULL}
};
static table_info_t gpkg_tile_matrix_set = {
  "gpkg_tile_matrix_set",
  gpkg_tile_matrix_set_columns,
  NULL, 0
};

static column_info_t gpkg_tile_matrix_columns[] = {
  {"table_name", "TEXT", N, SQL_NOT_NULL | SQL_PRIMARY_KEY, "CONSTRAINT fk_table_name__gpkg_contents_table_name REFERENCES gpkg_contents(table_name)"},
  {"zoom_level", "INTEGER", N, SQL_NOT_NULL | SQL_PRIMARY_KEY, NULL},
  {"matrix_width", "INTEGER", N, SQL_NOT_NULL, NULL},
  {"matrix_height", "INTEGER", N, SQL_NOT_NULL, NULL},
  {"tile_width", "INTEGER", N, SQL_NOT_NULL, NULL},
  {"tile_height", "INTEGER", N, SQL_NOT_NULL, NULL},
  {"pixel_x_size", "DOUBLE", N, SQL_NOT_NULL, NULL},
  {"pixel_y_size", "DOUBLE", N, SQL_NOT_NULL, NULL},
  {NULL, NULL, N, 0, NULL}
};
static table_info_t gpkg_tile_matrix = {
  "gpkg_tile_matrix",
  gpkg_tile_matrix_columns,
  NULL, 0
};

static const column_info_t gpkg_tiles_table_columns[] = {
  {"id", "INTEGER", N, SQL_PRIMARY_KEY | SQL_AUTOINCREMENT, NULL},
  {"zoom_level", "INTEGER", N, SQL_NOT_NULL | SQL_UNIQUE(1), NULL},
  {"tile_column", "INTEGER", N, SQL_NOT_NULL | SQL_UNIQUE(1), NULL},
  {"tile_row", "INTEGER", N, SQL_NOT_NULL | SQL_UNIQUE(1), NULL},
  {"tile_data", "BLOB", N, SQL_NOT_NULL, NULL},
  {NULL, NULL, N, 0, NULL}
};

static column_info_t gpkg_data_columns_columns[] = {
  {"table_name", "TEXT", N, SQL_NOT_NULL | SQL_PRIMARY_KEY, "CONSTRAINT fk_table_name__gpkg_contents_table_name REFERENCES gpkg_contents(table_name)"},
  {"column_name", "TEXT", N, SQL_NOT_NULL | SQL_PRIMARY_KEY, NULL},
  {"name", "TEXT", N, 0, NULL},
  {"title", "TEXT", N, 0, NULL},
  {"description", "TEXT", N, 0, NULL},
  {"mime_type", "TEXT", N, 0, NULL},
  {"constraint_name", "TEXT", N, 0, NULL},
  {NULL, NULL, N, 0, NULL}
};
static table_info_t gpkg_data_columns = {
  "gpkg_data_columns",
  gpkg_data_columns_columns,
  NULL, 0
};

static column_info_t gpkg_data_column_constraints_columns[] = {
  {"constraint_name", "TEXT", N, SQL_NOT_NULL | SQL_UNIQUE(1), NULL},
  {"constraint_type", "TEXT", N, SQL_NOT_NULL | SQL_UNIQUE(1), NULL},
  {"value", "TEXT", N, SQL_UNIQUE(1), NULL},
  {"min", "NUMERIC", N, 0, NULL},
  {"min_is_inclusive", "BOOLEAN", N, 0, NULL},
  {"max", "NUMERIC", N, 0, NULL},
  {"max_is_inclusive", "BOOLEAN", N, 0, NULL},
  {"description", "TEXT", N, 0, NULL},
  {NULL, NULL, N, 0, NULL}
};
static table_info_t gpkg_data_column_constraints = {
  "gpkg_data_column_constraints",
  gpkg_data_column_constraints_columns,
  NULL, 0
};

static column_info_t gpkg_metadata_columns[] = {
  {"id", "INTEGER", N, SQL_NOT_NULL | SQL_AUTOINCREMENT | SQL_PRIMARY_KEY, NULL},
  {"md_scope", "TEXT", T("dataset"), SQL_NOT_NULL, NULL},
  {"md_standard_uri", "TEXT", N, SQL_NOT_NULL, NULL},
  {"mime_type", "TEXT", T("text/xml"), SQL_NOT_NULL, NULL},
  {"metadata", "TEXT", N, SQL_NOT_NULL, NULL},
  {NULL, NULL, N, 0, NULL}
};
static table_info_t gpkg_metadata = {
  "gpkg_metadata",
  gpkg_metadata_columns,
  NULL, 0
};

static column_info_t gpkg_metadata_reference_columns[] = {
  {"reference_scope", "TEXT", N, SQL_NOT_NULL, NULL},
  {"table_name", "TEXT", N, 0, NULL},
  {"column_name", "TEXT", N, 0, NULL},
  {"row_id_value", "INTEGER", N, 0, NULL},
  {"timestamp", "DATETIME", F("strftime('%%Y-%%m-%%dT%%H:%%M:%%fZ',CURRENT_TIMESTAMP)"), SQL_NOT_NULL, NULL},
  {"md_file_id", "INTEGER", N, SQL_NOT_NULL, "CONSTRAINT fk_file_id__metadata_id REFERENCES gpkg_metadata(id)"},
  {"md_parent_id", "INTEGER", N, 0, "CONSTRAINT fk_parent_id__metadata_id REFERENCES gpkg_metadata(id)"},
  {NULL, NULL, N, 0, NULL}
};
static table_info_t gpkg_metadata_reference = {
  "gpkg_metadata_reference",
  gpkg_metadata_reference_columns,
  NULL, 0
};

static const table_info_t *const gpkg_tables[] = {
  &gpkg_contents,
  &gpkg_extensions,
  &gpkg_spatial_ref_sys,
  &gpkg_data_columns,
  &gpkg_data_column_constraints,
  &gpkg_metadata,
  &gpkg_metadata_reference,
  &gpkg_geometry_columns,
  &gpkg_tile_matrix_set,
  &gpkg_tile_matrix,
  NULL
};

static int init(sqlite3 *db, const char *db_name, errorstream_t *error) {
  int result = SQLITE_OK;
  const table_info_t *const *table = gpkg_tables;

  result = sql_exec(db, "PRAGMA application_id = %d", 0x47503130);
  if (result != SQLITE_OK) {
    error_append(error, "Could not set application_id");
  }

  if (result == SQLITE_OK) {
    while (*table != NULL) {
      result = sql_init_table(db, db_name, *table, error);
      if (result != SQLITE_OK) {
        break;
      }
      table++;
    }
  }

  if (result == SQLITE_OK && error_count(error) > 0) {
    result = SQLITE_ERROR;
  }

  return result;
}

/*
 * Check that each 'features' table in gpkg_contents has at least one corresponding row in gpkg_geometry_columns.
 * This is not covered by a foreign key reference.
 */
static int gpkg_contents_geometry_table_check_row(sqlite3 *db, sqlite3_stmt *stmt, void *data) {
  error_append(
    (errorstream_t *)data,
    "gpkg_contents: table '%s' has data_type 'features' but no rows exist in gpkg_geometry_columns for this table",
    sqlite3_column_text(stmt, 0)
  );
  return SQLITE_OK;
}

static int gpkg_contents_geometry_table_check(sqlite3 *db, const char *db_name, errorstream_t *error) {
  int result = sql_exec_stmt(
                 db, gpkg_contents_geometry_table_check_row, NULL, error,
                 "SELECT table_name FROM \"%w\".gpkg_contents WHERE data_type='features' AND table_name NOT IN (SELECT table_name FROM \"%w\".gpkg_geometry_columns)",
                 db_name, db_name
               );

  if (result != SQLITE_OK) {
    error_append(error, sqlite3_errmsg(db));
  }

  return result;
}

/*
 * Check that each 'tiles' table in gpkg_contents has at least one corresponding row in gpkg_tile_matrix_set.
 * This is not covered by a foreign key reference.
 */
static int gpkg_contents_tilemetadata_table_check_row(sqlite3 *db, sqlite3_stmt *stmt, void *data) {
  error_append(
    (errorstream_t *)data,
    "gpkg_contents: table '%s' has data_type 'tiles' but no rows exist in gpkg_tile_matrix_set for this table",
    sqlite3_column_text(stmt, 0)
  );
  return SQLITE_OK;
}

static int gpkg_contents_tilemetadata_table_check(sqlite3 *db, const char *db_name, errorstream_t *error) {
  int result = sql_exec_stmt(
                 db, gpkg_contents_tilemetadata_table_check_row, NULL, error,
                 "SELECT table_name FROM \"%w\".gpkg_contents WHERE data_type='tiles' AND table_name NOT IN (SELECT table_name FROM \"%w\".gpkg_tile_matrix_set)",
                 db_name, db_name
               );

  if (result != SQLITE_OK) {
    error_append(error, sqlite3_errmsg(db));
  }

  return result;
}

/*
 * Check that each meta tables that reference columns all refer to tables and columns that actually exist.
 */
typedef struct {
  const char *db_name;
  const char *source_table;
  errorstream_t *error;
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

static int gpkg_table_column_check(sqlite3 *db, const char *db_name, const char *table_name, const char *table_column, const char *column_column, errorstream_t *error) {
  int result = SQLITE_OK;
  column_check_data_t c;
  c.db_name = db_name;
  c.source_table = table_name;
  c.error = error;

  if (column_column != NULL) {
    result = sql_exec_stmt(db, gpkg_table_column_check_row, NULL, &c, "SELECT \"%w\", \"%w\" FROM \"%w\".\"%w\"", table_column, column_column, db_name, table_name);
  } else {
    result = sql_exec_stmt(db, gpkg_table_column_check_row, NULL, &c, "SELECT \"%w\", NULL FROM \"%w\".\"%w\"", table_column, db_name, table_name);
  }

  if (result != SQLITE_OK) {
    error_append(error, sqlite3_errmsg(db));
  }

  return result;
}

static int gpkg_contents_columns_table_column_check(sqlite3 *db, const char *db_name, errorstream_t *error) {
  return gpkg_table_column_check(db, db_name, "gpkg_contents", "table_name", NULL, error);
}

static int gpkg_extensions_table_column_check(sqlite3 *db, const char *db_name, errorstream_t *error) {
  return gpkg_table_column_check(db, db_name, "gpkg_extensions", "table_name", "column_name", error);
}

static int gpkg_geometry_columns_table_column_check(sqlite3 *db, const char *db_name, errorstream_t *error) {
  return gpkg_table_column_check(db, db_name, "gpkg_geometry_columns", "table_name", "column_name", error);
}

static int gpkg_tile_matrix_table_column_check(sqlite3 *db, const char *db_name, errorstream_t *error) {
  return gpkg_table_column_check(db, db_name, "gpkg_tile_matrix", "table_name", NULL, error);
}

static int gpkg_tile_matrix_set_table_column_check(sqlite3 *db, const char *db_name, errorstream_t *error) {
  return gpkg_table_column_check(db, db_name, "gpkg_tile_matrix_set", "table_name", NULL, error);
}

static int gpkg_data_columns_table_column_check(sqlite3 *db, const char *db_name, errorstream_t *error) {
  return gpkg_table_column_check(db, db_name, "gpkg_data_columns", "table_name", "column_name", error);
}

static int gpkg_metadata_reference_table_column_check(sqlite3 *db, const char *db_name, errorstream_t *error) {
  return gpkg_table_column_check(db, db_name, "gpkg_metadata_reference", "table_name", "column_name", error);
}

typedef int(*check_func)(sqlite3 *db, const char *db_name, errorstream_t *error);

static check_func checks[] = {
  gpkg_contents_columns_table_column_check,
  gpkg_extensions_table_column_check,
  gpkg_geometry_columns_table_column_check,
  gpkg_tile_matrix_table_column_check,
  gpkg_tile_matrix_set_table_column_check,
  gpkg_data_columns_table_column_check,
  gpkg_metadata_reference_table_column_check,
  gpkg_contents_geometry_table_check,
  gpkg_contents_tilemetadata_table_check,
  NULL
};

static int check(sqlite3 *db, const char *db_name, int flags, errorstream_t *error) {
  int result = SQLITE_OK;

  if (result == SQLITE_OK) {
    result = sql_check_table(db, db_name, &gpkg_contents, SQL_MUST_EXIST | flags, error);
  }
  if (result == SQLITE_OK) {
    result = sql_check_table(db, db_name, &gpkg_spatial_ref_sys, SQL_MUST_EXIST | flags, error);
  }
  if (result == SQLITE_OK) {
    result = sql_check_table(db, db_name, &gpkg_extensions, flags, error);
  }
  if (result == SQLITE_OK) {
    result = sql_check_table(db, db_name, &gpkg_data_columns, flags, error);
  }
  if (result == SQLITE_OK) {
    result = sql_check_table(db, db_name, &gpkg_data_column_constraints, flags, error);
  }
  if (result == SQLITE_OK) {
    result = sql_check_table(db, db_name, &gpkg_metadata, flags, error);
  }
  if (result == SQLITE_OK) {
    result = sql_check_table(db, db_name, &gpkg_metadata_reference, flags, error);
  }
  if (result == SQLITE_OK) {
    int features = 0;
    result = sql_exec_for_int(db, &features, "SELECT count(*) FROM \"%w\".gpkg_contents WHERE data_type LIKE 'features'", db_name);
    if (result == SQLITE_OK) {
      result = sql_check_table(db, db_name, &gpkg_geometry_columns, flags | (features > 0 ? SQL_MUST_EXIST : 0), error);
    }
  }
  if (result == SQLITE_OK) {
    int tiles = 0;
    result = sql_exec_for_int(db, &tiles, "SELECT count(*) FROM \"%w\".gpkg_contents WHERE data_type LIKE 'tiles'", db_name);
    if (result == SQLITE_OK) {
      result = sql_check_table(db, db_name, &gpkg_tile_matrix_set, flags | (tiles > 0 ? SQL_MUST_EXIST : 0), error);
      result = sql_check_table(db, db_name, &gpkg_tile_matrix, flags | (tiles > 0 ? SQL_MUST_EXIST : 0), error);
    }
  }

  if ((flags & SQL_CHECK_ALL_DATA) != 0) {
    if (result == SQLITE_OK) {
      result = sql_check_integrity(db, db_name, error);
    }
    if (result == SQLITE_OK) {
      check_func *current_func = checks;
      while (*current_func != NULL) {
        result = (*current_func)(db, db_name, error);
        if (result != SQLITE_OK) {
          break;
        }
        current_func++;
      }
    }
  }

  return result;
}

static int write_blob_header(binstream_t *stream, geom_blob_header_t *header, errorstream_t *error) {
  return gpb_write_header(stream, header, error);
}

static int read_blob_header(binstream_t *stream, geom_blob_header_t *header, errorstream_t *error) {
  return gpb_read_header(stream, header, error);
}

static int gpkg_writer_init(geom_blob_writer_t *writer) {
  return gpb_writer_init(writer, -1);
}

static int add_geometry_column(sqlite3 *db, const char *db_name, const char *table_name, const char *column_name, const char *geom_type, int srs_id, int z, int m, errorstream_t *error) {
  int result;

  const char *normalized_geom_type;
  result = geom_normalized_type_name(geom_type, &normalized_geom_type);
  if (result != SQLITE_OK) {
    error_append(error, "Invalid geometry type: %s", geom_type);
    return result;
  }

  if (z < 0 || z > 2) {
    error_append(error, "Invalid Z flag value: %d", z);
    return result;
  }

  if (m < 0 || m > 2) {
    error_append(error, "Invalid M flag value: %d", z);
    return result;
  }

  // Check if the target table exists
  int exists = 0;
  result = sql_check_table_exists(db, db_name, table_name, &exists);
  if (result != SQLITE_OK) {
    error_append(error, "Could not check if table %s.%s exists", db_name, table_name);
    return result;
  }

  if (!exists) {
    error_append(error, "Table %s.%s does not exist", db_name, table_name);
    return SQLITE_OK;
  }

  if (error_count(error) > 0) {
    return SQLITE_OK;
  }

  // Check if the SRID is defined
  int count = 0;
  result = sql_exec_for_int(db, &count, "SELECT count(*) FROM gpkg_spatial_ref_sys WHERE srs_id = %d", srs_id);
  if (result != SQLITE_OK) {
    return result;
  }

  if (count == 0) {
    error_append(error, "SRS %d does not exist", srs_id);
    return SQLITE_OK;
  }

  result = sql_exec(db, "ALTER TABLE \"%w\".\"%w\" ADD COLUMN \"%w\" %s", db_name, table_name, column_name, normalized_geom_type);
  if (result != SQLITE_OK) {
    error_append(error, sqlite3_errmsg(db));
    return result;
  }

  result = sql_exec(db, "INSERT INTO \"%w\".\"%w\" (table_name, column_name, geometry_type_name, srs_id, z, m) VALUES (%Q, %Q, %Q, %d, %d, %d)", db_name, "gpkg_geometry_columns", table_name,
                    column_name,
                    normalized_geom_type, srs_id, z, m);
  if (result != SQLITE_OK) {
    error_append(error, sqlite3_errmsg(db));
    return result;
  }

  return SQLITE_OK;
}

static int create_tiles_table(sqlite3 *db, const char *db_name, const char *table_name, errorstream_t *error) {
  int result = SQLITE_OK;

  // Check if the target table exists
  int exists = 0;
  result = sql_check_table_exists(db, db_name, table_name, &exists);
  if (result != SQLITE_OK) {
    error_append(error, "Could not check if table %s.%s exists", db_name, table_name);
    return result;
  }

  if (exists) {
    error_append(error, "Table %s.%s already exists", db_name, table_name);
    return SQLITE_OK;
  }

  table_info_t tile_table_info = {
    table_name,
    gpkg_tiles_table_columns,
    NULL, 0
  };

  // Check if required meta tables exist
  result = sql_init_table(db, db_name, &tile_table_info, error);
  if (result != SQLITE_OK) {
    return result;
  }

  return SQLITE_OK;
}

static int create_spatial_index(sqlite3 *db, const char *db_name, const char *table_name, const char *geometry_column_name, const char *id_column_name, errorstream_t *error) {
  int result = SQLITE_OK;
  char *index_table_name = NULL;
  int exists = 0;

  index_table_name = sqlite3_mprintf("rtree_%s_%s", table_name, geometry_column_name);
  if (index_table_name == NULL) {
    result = SQLITE_NOMEM;
    goto exit;
  }

  // Check if the target table exists
  exists = 0;
  result = sql_check_table_exists(db, db_name, index_table_name, &exists);
  if (result != SQLITE_OK) {
    error_append(error, "Could not check if index table %s.%s exists: %s", db_name, index_table_name, sqlite3_errmsg(db));
    goto exit;
  }

  if (exists) {
    result = SQLITE_OK;
    goto exit;
  }

  // Check if the target table exists
  exists = 0;
  result = sql_check_table_exists(db, db_name, table_name, &exists);
  if (result != SQLITE_OK) {
    error_append(error, "Could not check if table %s.%s exists: %s", db_name, table_name, sqlite3_errmsg(db));
    goto exit;
  }

  if (!exists) {
    error_append(error, "Table %s.%s does not exist", db_name, table_name);
    goto exit;
  }

  int geom_col_count = 0;
  result = sql_exec_for_int(db, &geom_col_count, "SELECT count(*) FROM \"%w\".gpkg_geometry_columns WHERE table_name LIKE %Q AND column_name LIKE %Q", db_name, table_name, geometry_column_name);
  if (result != SQLITE_OK) {
    error_append(error, "Could not check if column %s.%s.%s exists in %s.gpkg_geometry_columns: %s", db_name, table_name, geometry_column_name, db_name, sqlite3_errmsg(db));
    goto exit;
  }

  if (geom_col_count == 0) {
    error_append(error, "Column %s.%s.%s is not registered in %s.gpkg_geometry_columns", db_name, table_name, geometry_column_name, db_name);
    goto exit;
  }

  result = sql_exec(db, "CREATE VIRTUAL TABLE \"%w\".\"%w\" USING rtree(id, minx, maxx, miny, maxy)", db_name, index_table_name);
  if (result != SQLITE_OK) {
    error_append(error, "Could not create rtree table %s.%s: %s", db_name, index_table_name, sqlite3_errmsg(db));
    goto exit;
  }

  result = sql_exec(
             db,
             "CREATE TRIGGER \"%w\".\"rtree_%w_%w_insert\" AFTER INSERT ON \"%w\"\n"
             "    WHEN (NEW.\"%w\" NOTNULL AND NOT ST_IsEmpty(NEW.\"%w\"))\n"
             "BEGIN\n"
             "  INSERT OR REPLACE INTO \"%w\" VALUES (\n"
             "    NEW.\"%w\",\n"
             "    ST_MinX(NEW.\"%w\"), ST_MaxX(NEW.\"%w\"),\n"
             "    ST_MinY(NEW.\"%w\"), ST_MaxY(NEW.\"%w\")\n"
             "  );\n"
             "END;",
             db_name, table_name, geometry_column_name, table_name,
             geometry_column_name, geometry_column_name,
             index_table_name,
             id_column_name,
             geometry_column_name, geometry_column_name,
             geometry_column_name, geometry_column_name
           );
  if (result != SQLITE_OK) {
    error_append(error, "Could not create rtree insert trigger: %s", sqlite3_errmsg(db));
    goto exit;
  }

  result = sql_exec(
             db,
             "CREATE TRIGGER \"%w\".\"rtree_%w_%w_update1\" AFTER UPDATE OF \"%w\" ON \"%w\"\n"
             "    WHEN OLD.\"%w\" = NEW.\"%w\" AND\n"
             "         (NEW.\"%w\" NOTNULL AND NOT ST_IsEmpty(NEW.\"%w\"))\n"
             "BEGIN\n"
             "  INSERT OR REPLACE INTO \"%w\" VALUES (\n"
             "    NEW.\"%w\",\n"
             "    ST_MinX(NEW.\"%w\"), ST_MaxX(NEW.\"%w\"),\n"
             "    ST_MinY(NEW.\"%w\"), ST_MaxY(NEW.\"%w\")\n"
             "  );\n"
             "END;",
             db_name, table_name, geometry_column_name, geometry_column_name, table_name,
             id_column_name, id_column_name,
             geometry_column_name, geometry_column_name,
             index_table_name,
             id_column_name,
             geometry_column_name, geometry_column_name,
             geometry_column_name, geometry_column_name
           );
  if (result != SQLITE_OK) {
    error_append(error, "Could not create rtree update trigger 1: %s", sqlite3_errmsg(db));
    goto exit;
  }

  result = sql_exec(
             db,
             "CREATE TRIGGER \"%w\".\"rtree_%w_%w_update2\" AFTER UPDATE OF \"%w\" ON \"%w\"\n"
             "    WHEN OLD.\"%w\" = NEW.\"%w\" AND\n"
             "         (NEW.\"%w\" ISNULL OR ST_IsEmpty(NEW.\"%w\"))\n"
             "BEGIN\n"
             "  DELETE FROM \"%w\" WHERE id = OLD.\"%w\";\n"
             "END;",
             db_name, table_name, geometry_column_name, geometry_column_name, table_name,
             id_column_name, id_column_name,
             geometry_column_name, geometry_column_name,
             index_table_name, id_column_name
           );
  if (result != SQLITE_OK) {
    error_append(error, "Could not create rtree update trigger 2: %s", sqlite3_errmsg(db));
    goto exit;
  }

  result = sql_exec(
             db,
             "CREATE TRIGGER \"%w\".\"rtree_%w_%w_update3\" AFTER UPDATE ON \"%w\"\n"
             "    WHEN OLD.\"%w\" != NEW.\"%w\" AND\n"
             "         (NEW.\"%w\" NOTNULL AND NOT ST_IsEmpty(NEW.\"%w\"))\n"
             "BEGIN\n"
             "  DELETE FROM \"%w\" WHERE id = OLD.\"%w\";\n"
             "  INSERT OR REPLACE INTO \"%w\" VALUES (\n"
             "    NEW.\"%w\",\n"
             "    ST_MinX(NEW.\"%w\"), ST_MaxX(NEW.\"%w\"),\n"
             "    ST_MinY(NEW.\"%w\"), ST_MaxY(NEW.\"%w\")\n"
             "  );\n"
             "END;",
             db_name, table_name, geometry_column_name, table_name,
             id_column_name, id_column_name,
             geometry_column_name, geometry_column_name,
             index_table_name, id_column_name,
             index_table_name,
             id_column_name,
             geometry_column_name, geometry_column_name,
             geometry_column_name, geometry_column_name
           );
  if (result != SQLITE_OK) {
    error_append(error, "Could not create rtree update trigger 3: %s", sqlite3_errmsg(db));
    goto exit;
  }

  result = sql_exec(
             db,
             "CREATE TRIGGER \"%w\".\"rtree_%w_%w_update4\" AFTER UPDATE ON \"%w\"\n"
             "    WHEN OLD.\"%w\" != NEW.\"%w\" AND\n"
             "         (NEW.\"%w\" ISNULL OR ST_IsEmpty(NEW.\"%w\"))\n"
             "BEGIN\n"
             "  DELETE FROM \"%w\" WHERE id IN (OLD.\"%w\", NEW.\"%w\");\n"
             "END;",
             db_name, table_name, geometry_column_name, table_name,
             id_column_name, id_column_name,
             geometry_column_name, geometry_column_name,
             index_table_name, id_column_name, id_column_name
           );
  if (result != SQLITE_OK) {
    error_append(error, "Could not create rtree update trigger 4: %s", sqlite3_errmsg(db));
    goto exit;
  }

  result = sql_exec(
             db,
             "CREATE TRIGGER \"%w\".\"rtree_%w_%w_delete\" AFTER DELETE ON \"%w\"\n"
             "BEGIN\n"
             "  DELETE FROM \"%w\" WHERE id = OLD.\"%w\";\n"
             "END;",
             db_name, table_name, geometry_column_name, table_name,
             index_table_name, id_column_name
           );
  if (result != SQLITE_OK) {
    error_append(error, "Could not create rtree delete trigger: %s", sqlite3_errmsg(db));
    goto exit;
  }

  result = sql_exec(
             db,
             "INSERT OR REPLACE INTO \"%w\".\"%w\" (id, minx, maxx, miny, maxy) "
             "  SELECT \"%w\", ST_MinX(\"%w\"), ST_MaxX(\"%w\"), ST_MinY(\"%w\"), ST_MaxY(\"%w\") FROM \"%w\".\"%w\""
             "  WHERE \"%w\" NOTNULL AND NOT ST_IsEmpty(\"%w\")",
             db_name, index_table_name,
             id_column_name, geometry_column_name, geometry_column_name, geometry_column_name, geometry_column_name, db_name, table_name,
             geometry_column_name, geometry_column_name
           );
  if (result != SQLITE_OK) {
    error_append(error, "Could not populate rtree: %s", sqlite3_errmsg(db));
    goto exit;
  }

  result = sql_exec(
             db,
             "INSERT OR REPLACE INTO \"%w\".\"gpkg_extensions\" (table_name, column_name, extension_name, definition, scope) VALUES (\"%w\", \"%w\", \"%w\", \"%w\", \"%w\")",
             db_name, table_name, geometry_column_name, "gpkg_rtree_index", "GeoPackage 1.0 Specification Annex L", "write-only"
           );
  if (result != SQLITE_OK) {
    error_append(error, "Could not register rtree usage in gpkg_extensions: %s", sqlite3_errmsg(db));
    goto exit;
  }

exit:
  sqlite3_free(index_table_name);
  return result;
}

static int fill_envelope(binstream_t *stream, geom_envelope_t *envelope, errorstream_t *error) {
  return wkb_fill_envelope(stream, WKB_ISO, envelope, error);
}

static int read_geometry_header(binstream_t *stream, geom_header_t *header, errorstream_t *error) {
  return wkb_read_header(stream, WKB_ISO, header, error);
}

static int read_geometry(binstream_t *stream, geom_consumer_t const *consumer, errorstream_t *error) {
  return wkb_read_geometry(stream, WKB_ISO, consumer, error);
}

static const spatialdb_t GEOPACKAGE = {
  "GeoPackage",
  NULL,
  init,
  check,
  write_blob_header,
  read_blob_header,
  gpkg_writer_init,
  gpb_writer_init,
  gpb_writer_destroy,
  add_geometry_column,
  create_tiles_table,
  create_spatial_index,
  fill_envelope,
  read_geometry_header,
  read_geometry
};

const spatialdb_t *spatialdb_geopackage_schema() {
  return &GEOPACKAGE;
}
