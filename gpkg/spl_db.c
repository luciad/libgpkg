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
#include "check.h"
#include "spatialdb.h"
#include "spl_geom.h"
#include "sqlite.h"

#define N NULL_VALUE
#define D(v) DOUBLE_VALUE(v)
#define I(v) INT_VALUE(v)
#define T(v) TEXT_VALUE(v)
#define F(v) FUNC_VALUE(v)

static column_info_t spl3_spatial_ref_sys_columns[] = {
  {"srid", "integer", N, SQL_NOT_NULL | SQL_PRIMARY_KEY, NULL},
  {"auth_name", "text", N, SQL_NOT_NULL | SQL_UNIQUE(1), NULL},
  {"auth_srid", "integer", N, SQL_NOT_NULL | SQL_UNIQUE(1), NULL},
  {"ref_sys_name", "text", T("Unknown"), SQL_NOT_NULL, NULL},
  {"proj4text", "text", N, SQL_NOT_NULL, NULL},
  {"srs_wkt", "text", T("Undefined"), SQL_NOT_NULL, NULL},
  {NULL, NULL, N, 0, NULL}
};
static value_t spl3_spatial_ref_sys_data[] = {
  I(-1), T("NONE"), I(-1), T("Undefined - Cartesian"), T(""), T("Undefined"),
  I(0), T("NONE"), I(0), T("Undefined - Geographic Long/Lat"), T(""), T("Undefined")
};
static table_info_t spl3_spatial_ref_sys = {
  "spatial_ref_sys",
  spl3_spatial_ref_sys_columns,
  spl3_spatial_ref_sys_data, 2
};

static column_info_t spl3_geometry_columns_columns[] = {
  {"f_table_name", "text", N, SQL_NOT_NULL | SQL_PRIMARY_KEY, NULL},
  {"f_geometry_column", "text", N, SQL_NOT_NULL | SQL_PRIMARY_KEY, NULL},
  {"type", "text", N, SQL_NOT_NULL, NULL},
  {"coord_dimension", "text", N, SQL_NOT_NULL, NULL},
  {"srid", "integer", N, SQL_NOT_NULL, "CONSTRAINT fk_srs_id__spatial_ref_sys_srs_id REFERENCES spatial_ref_sys(srs_id)"},
  {"spatial_index_enabled", "integer", N, SQL_NOT_NULL, NULL},
  {NULL, NULL, N, 0, NULL}
};
static table_info_t spl3_geometry_columns = {
  "geometry_columns",
  spl3_geometry_columns_columns,
  NULL, 0
};

static const table_info_t *const spl3_tables[] = {
  &spl3_spatial_ref_sys,
  &spl3_geometry_columns,
  NULL
};

static column_info_t spl4_spatial_ref_sys_columns[] = {
  {"srid", "integer", N, SQL_NOT_NULL | SQL_PRIMARY_KEY, NULL},
  {"auth_name", "text", N, SQL_NOT_NULL | SQL_UNIQUE(1), NULL},
  {"auth_srid", "integer", N, SQL_NOT_NULL | SQL_UNIQUE(1), NULL},
  {"ref_sys_name", "text", T("Unknown"), SQL_NOT_NULL, NULL},
  {"proj4text", "text", N, SQL_NOT_NULL, NULL},
  {"srtext", "text", T("Undefined"), SQL_NOT_NULL, NULL},
  {NULL, NULL, N, 0, NULL}
};
static value_t spl4_spatial_ref_sys_data[] = {
  I(-1), T("NONE"), I(-1), T("Undefined - Cartesian"), T(""), T("Undefined"),
  I(0), T("NONE"), I(0), T("Undefined - Geographic Long/Lat"), T(""), T("Undefined")
};
static table_info_t spl4_spatial_ref_sys = {
  "spatial_ref_sys",
  spl4_spatial_ref_sys_columns,
  spl4_spatial_ref_sys_data, 2
};

static column_info_t spl4_geometry_columns_columns[] = {
  {"f_table_name", "text", N, SQL_NOT_NULL | SQL_PRIMARY_KEY, NULL},
  {"f_geometry_column", "text", N, SQL_NOT_NULL | SQL_PRIMARY_KEY, NULL},
  {"geometry_type", "integer", N, SQL_NOT_NULL, NULL},
  {"coord_dimension", "integer", N, SQL_NOT_NULL, NULL},
  {"srid", "integer", N, SQL_NOT_NULL, "CONSTRAINT fk_srs_id__spatial_ref_sys_srs_id REFERENCES spatial_ref_sys(srs_id)"},
  {"spatial_index_enabled", "integer", N, SQL_NOT_NULL, NULL},
  {NULL, NULL, N, 0, NULL}
};
static table_info_t spl4_geometry_columns = {
  "geometry_columns",
  spl4_geometry_columns_columns,
  NULL, 0
};

static const table_info_t *const spl4_tables[] = {
  &spl4_spatial_ref_sys,
  &spl4_geometry_columns,
  NULL
};

static int spl3_init(sqlite3 *db, const char *db_name, error_t *error) {
  return init_database(db, db_name, spl3_tables, error);
}

static int spl3_check(sqlite3 *db, const char *db_name, error_t *error) {
  return check_database(db, db_name, spl3_tables, error);
}

static int spl4_init(sqlite3 *db, const char *db_name, error_t *error) {
  return init_database(db, db_name, spl4_tables, error);
}

static int spl4_check(sqlite3 *db, const char *db_name, error_t *error) {
  return check_database(db, db_name, spl4_tables, error);
}

static int write_blob_header(binstream_t *stream, geom_blob_header_t *header, error_t *error) {
  return spb_write_header(stream, header, error);
}

static int read_blob_header(binstream_t *stream, geom_blob_header_t *header, error_t *error) {
  return spb_read_header(stream, header, error);
}

static int spl3_writer_init(geom_blob_writer_t *writer) {
  return spb_writer_init(writer, -1);
}

static int spl3_add_geometry_column(sqlite3 *db, const char *db_name, const char *table_name, const char *column_name, const char *geom_type, int srs_id, int z, int m, error_t *error) {
  int result;
  geom_type_t geom_type_enum;

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

  if (z == 2) {
    error_append(error, "Optional Z values are not supported by Spatialite");
    return result;
  }

  if (m == 2) {
    error_append(error, "Optional M values are not supported by Spatialite");
    return result;
  }

  const char *coord_type;
  if (z == 1 && m == 1) {
    coord_type = "XYZM";
  } else if (z == 0 && m == 1) {
    coord_type = "XYM";
  } else if (z == 1 && m == 0) {
    coord_type = "XYZ";
  } else {
    coord_type = "XY";
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
  result = sql_exec_for_int(db, &count, "SELECT count(*) FROM spatial_ref_sys WHERE srid = %d", srs_id);
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

  result = sql_exec(db, "INSERT INTO \"%w\".\"%w\" (f_table_name, f_geometry_column, geometry_type, coord_dimension, srid, spatial_index_enabled) VALUES (%Q, %Q, %Q, %Q, %d, %d)", db_name,
                    "geometry_columns", table_name, column_name,
                    normalized_geom_type, coord_type, srs_id, 0);
  if (result != SQLITE_OK) {
    error_append(error, sqlite3_errmsg(db));
    return result;
  }

  return SQLITE_OK;
}

static int spl4_writer_init(geom_blob_writer_t *writer) {
  return spb_writer_init(writer, 0);
}

static int spl4_add_geometry_column(sqlite3 *db, const char *db_name, const char *table_name, const char *column_name, const char *geom_type, int srs_id, int z, int m, error_t *error) {
  int result;
  geom_type_t geom_type_enum;

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

  if (z == 2) {
    error_append(error, "Optional Z values are not supported by Spatialite");
    return result;
  }

  if (m == 2) {
    error_append(error, "Optional M values are not supported by Spatialite");
    return result;
  }

  int geom_type_code = geom_type_enum;
  coord_type_t coord_type;
  if (z == 1 && m == 1) {
    coord_type = GEOM_XYZM;
    geom_type_code += 3000;
  } else if (z == 0 && m == 1) {
    coord_type = GEOM_XYM;
    geom_type_code += 2000;
  } else if (z == 1 && m == 0) {
    coord_type = GEOM_XYZ;
    geom_type_code += 1000;
  } else {
    coord_type = GEOM_XY;
  }
  int coord_dim = geom_coord_dim(coord_type);

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
  result = sql_exec_for_int(db, &count, "SELECT count(*) FROM spatial_ref_sys WHERE srid = %d", srs_id);
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

  result = sql_exec(db, "INSERT INTO \"%w\".\"%w\" (f_table_name, f_geometry_column, geometry_type, coord_dimension, srid, spatial_index_enabled) VALUES (%Q, %Q, %d, %d, %d, %d)", db_name,
                    "geometry_columns", table_name, column_name,
                    geom_type_code, coord_dim, srs_id, 0);
  if (result != SQLITE_OK) {
    error_append(error, sqlite3_errmsg(db));
    return result;
  }

  return SQLITE_OK;
}

static int fill_envelope(binstream_t *stream, geom_envelope_t *envelope, error_t *error) {
  return wkb_fill_envelope(stream, WKB_SPATIALITE, envelope, error);
}

static int read_geometry_header(binstream_t *stream, geom_header_t *header, error_t *error) {
  return wkb_read_header(stream, WKB_SPATIALITE, header, error);
}

static int read_geometry(binstream_t *stream, geom_consumer_t const *consumer, error_t *error) {
  return wkb_read_geometry(stream, WKB_SPATIALITE, consumer, error);
}

/*
 * (geometry blob, geometry_type text, srid int, dimension text);
 * (geometry blob, geometry_type int, srid int);
 */
static int geometry_constraints(sqlite3_context *context, int nbArgs, sqlite3_value **args) {

}

#ifdef __cplusplus
extern "C" {
#endif

const spatialdb_t SPATIALITE3_DB = {
  "Spatialite3",
  spl3_init,
  spl3_check,
  write_blob_header,
  read_blob_header,
  spl3_writer_init,
  spb_writer_init,
  spb_writer_destroy,
  spl3_add_geometry_column,
  NULL,
  NULL,
  fill_envelope,
  read_geometry_header,
  read_geometry
};

const spatialdb_t SPATIALITE4_DB = {
  "Spatialite4",
  spl4_init,
  spl4_check,
  write_blob_header,
  read_blob_header,
  spl4_writer_init,
  spb_writer_init,
  spb_writer_destroy,
  spl4_add_geometry_column,
  NULL,
  NULL,
  fill_envelope,
  read_geometry_header,
  read_geometry
};

#ifdef __cplusplus
}
#endif