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
#include "spatialdb_internal.h"
#include "spl_geom.h"
#include "sql.h"
#include "sqlite.h"
#include "blobio.h"
#include "geomio.h"

#define N NULL_VALUE
#define D(v) DOUBLE_VALUE(v)
#define I(v) INT_VALUE(v)
#define T(v) TEXT_VALUE(v)
#define F(v) FUNC_VALUE(v)

static column_info_t spl2_spatial_ref_sys_columns[] = {
  {"srid", "integer", N, SQL_NOT_NULL | SQL_PRIMARY_KEY, NULL},
  {"auth_name", "varchar(256)", N, SQL_NOT_NULL, NULL},
  {"auth_srid", "integer", N, SQL_NOT_NULL, NULL},
  {"ref_sys_name", "varchar(256)", T("Unknown"), 0, NULL},
  {"proj4text", "varchar(2048)", N, SQL_NOT_NULL, NULL},
  {NULL, NULL, N, 0, NULL}
};
static value_t spl2_spatial_ref_sys_data[] = {
  I(-1), T("NONE"), I(-1), T("Undefined - Cartesian"), T(""),
  I(0), T("NONE"), I(0), T("Undefined - Geographic Long/Lat"), T("")
};
static table_info_t spl2_spatial_ref_sys = {
  "spatial_ref_sys",
  spl2_spatial_ref_sys_columns,
  spl2_spatial_ref_sys_data, 2
};

static column_info_t spl2_geometry_columns_columns[] = {
  {"f_table_name", "varchar(256)", N, SQL_NOT_NULL, NULL},
  {"f_geometry_column", "varchar(256)", N, SQL_NOT_NULL, NULL},
  {"type", "varchar(30)", N, SQL_NOT_NULL, NULL},
  {"coord_dimension", "integer", N, SQL_NOT_NULL, NULL},
  {"srid", "integer", N, 0, NULL},
  {"spatial_index_enabled", "integer", N, SQL_NOT_NULL, NULL},
  {NULL, NULL, N, 0, NULL}
};
static table_info_t spl2_geometry_columns = {
  "geometry_columns",
  spl2_geometry_columns_columns,
  NULL, 0
};

static const table_info_t *const spl2_tables[] = {
  &spl2_spatial_ref_sys,
  &spl2_geometry_columns,
  NULL
};

static column_info_t spl3_spatial_ref_sys_columns[] = {
  {"srid", "integer", N, SQL_NOT_NULL | SQL_PRIMARY_KEY, NULL},
  {"auth_name", "text", N, SQL_NOT_NULL, NULL},
  {"auth_srid", "integer", N, SQL_NOT_NULL, NULL},
  {"ref_sys_name", "text", T("Unknown"), 0, NULL},
  {"proj4text", "text", N, SQL_NOT_NULL, NULL},
  {"srs_wkt", "text", T("Undefined"), 0, NULL},
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

static int spl2_init(sqlite3 *db, const char *db_name, errorstream_t *error) {
  int result = SQLITE_OK;
  const table_info_t *const *table = spl2_tables;

  while (*table != NULL) {
    result = sql_init_table(db, db_name, *table, error);
    if (result != SQLITE_OK) {
      break;
    }
    table++;
  }

  if (result == SQLITE_OK && error_count(error) > 0) {
    return SQLITE_ERROR;
  } else {
    return result;
  }
}

static int spl2_check(sqlite3 *db, const char *db_name, int check_flags, errorstream_t *error) {
  int result = SQLITE_OK;

  const table_info_t *const *table = spl2_tables;
  while (*table != NULL) {
    result = sql_check_table(db, db_name, *table, check_flags | SQL_MUST_EXIST, error);
    if (result != SQLITE_OK) {
      break;
    }
    table++;
  }

  return result;
}

static int spl2_add_geometry_column(sqlite3 *db, const char *db_name, const char *table_name, const char *column_name, const char *geom_type, int srs_id, int z, int m, errorstream_t *error) {
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

  if (z == 2) {
    error_append(error, "Optional Z values are not supported by Spatialite");
    return result;
  }

  if (m == 2) {
    error_append(error, "Optional M values are not supported by Spatialite");
    return result;
  }

  int coord_type;
  if (z == 1 && m == 1) {
    coord_type = 4;
  } else if (z == 0 && m == 1) {
    coord_type = 3;
  } else if (z == 1 && m == 0) {
    coord_type = 3;
  } else {
    coord_type = 2;
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

  result = sql_exec(db, "INSERT INTO \"%w\".\"%w\" (f_table_name, f_geometry_column, type, coord_dimension, srid, spatial_index_enabled) VALUES (%Q, %Q, %Q, %d, %d, %d)", db_name,
                    "geometry_columns", table_name, column_name,
                    normalized_geom_type, coord_type, srs_id, 0);
  if (result != SQLITE_OK) {
    error_append(error, sqlite3_errmsg(db));
    return result;
  }

  return SQLITE_OK;
}

static int spl3_init(sqlite3 *db, const char *db_name, errorstream_t *error) {
  int result = SQLITE_OK;
  const table_info_t *const *table = spl3_tables;

  while (*table != NULL) {
    result = sql_init_table(db, db_name, *table, error);
    if (result != SQLITE_OK) {
      break;
    }
    table++;
  }

  if (result == SQLITE_OK && error_count(error) > 0) {
    return SQLITE_ERROR;
  } else {
    return result;
  }
}

static int spl3_check(sqlite3 *db, const char *db_name, int check_flags, errorstream_t *error) {
  int result = SQLITE_OK;

  const table_info_t *const *table = spl3_tables;
  while (*table != NULL) {
    result = sql_check_table(db, db_name, *table, check_flags | SQL_MUST_EXIST, error);
    if (result != SQLITE_OK) {
      break;
    }
    table++;
  }

  return result;
}

static int spl4_init(sqlite3 *db, const char *db_name, errorstream_t *error) {
  int result = SQLITE_OK;
  const table_info_t *const *table = spl4_tables;

  while (*table != NULL) {
    result = sql_init_table(db, db_name, *table, error);
    if (result != SQLITE_OK) {
      break;
    }
    table++;
  }

  if (result == SQLITE_OK && error_count(error) > 0) {
    return SQLITE_ERROR;
  } else {
    return result;
  }
}

static int spl4_check(sqlite3 *db, const char *db_name, int check_flags, errorstream_t *error) {
  int result = SQLITE_OK;

  const table_info_t *const *table = spl4_tables;
  while (*table != NULL) {
    result = sql_check_table(db, db_name, *table, check_flags | SQL_MUST_EXIST, error);
    if (result != SQLITE_OK) {
      break;
    }
    table++;
  }

  return result;
}

static int write_blob_header(binstream_t *stream, geom_blob_header_t *header, errorstream_t *error) {
  return spb_write_header(stream, header, error);
}

static int read_blob_header(binstream_t *stream, geom_blob_header_t *header, errorstream_t *error) {
  return spb_read_header(stream, header, error);
}

static int spl3_writer_init(geom_blob_writer_t *writer) {
  return spb_writer_init(writer, -1);
}

static int spl3_add_geometry_column(sqlite3 *db, const char *db_name, const char *table_name, const char *column_name, const char *geom_type, int srs_id, int z, int m, errorstream_t *error) {
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

  result = sql_exec(db, "INSERT INTO \"%w\".\"%w\" (f_table_name, f_geometry_column, type, coord_dimension, srid, spatial_index_enabled) VALUES (%Q, %Q, %Q, %Q, %d, %d)", db_name,
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

static int spl4_add_geometry_column(sqlite3 *db, const char *db_name, const char *table_name, const char *column_name, const char *geom_type, int srs_id, int z, int m, errorstream_t *error) {
  int result;
  geom_type_t geom_type_enum;

  result = geom_type_from_string(geom_type, &geom_type_enum);
  if (result != SQLITE_OK) {
    error_append(error, "Invalid geometry type: %s", geom_type);
    return result;
  }

  const char *normalized_geom_type;
  result = geom_type_name(geom_type_enum, &normalized_geom_type);
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

  result = sql_exec(db, "DROP TRIGGER IF EXISTS \"%w\".\"ggi_%w_%w\"", db_name, table_name, column_name);
  if (result != SQLITE_OK) {
    error_append(error, "Could not drop old geometry insert trigger %s.ggi_%s_%s: %s", db_name, table_name, column_name, sqlite3_errmsg(db));
    return result;
  }
  result = sql_exec(db, "DROP TRIGGER IF EXISTS \"%w\".\"ggu_%w_%w\"", db_name, table_name, column_name);
  if (result != SQLITE_OK) {
    error_append(error, "Could not drop old geometry update trigger %s.ggu_%s_%s: %s", db_name, table_name, column_name, sqlite3_errmsg(db));
    return result;
  }

  result = sql_exec(
             db,
             "CREATE TRIGGER \"%w\".\"ggi_%w_%w\" AFTER INSERT ON \"%w\"\n"
             "BEGIN\n"
             "  SELECT GeometryConstraints(NEW.\"%w\", geometry_type, srid) FROM geometry_columns WHERE f_table_name LIKE %Q and f_geometry_column LIKE %Q;\n"
             "END;",
             db_name, table_name, column_name, table_name,
             column_name, table_name, column_name
           );
  if (result != SQLITE_OK) {
    error_append(error, "Could not create geometry insert trigger: %s", sqlite3_errmsg(db));
    return result;
  }

  result = sql_exec(
             db,
             "CREATE TRIGGER \"%w\".\"ggu_%w_%w\" AFTER UPDATE ON \"%w\"\n"
             "BEGIN\n"
             "  SELECT GeometryConstraints(NEW.\"%w\", geometry_type, srid) FROM geometry_columns WHERE f_table_name LIKE %Q and f_geometry_column LIKE %Q;\n"
             "END;",
             db_name, table_name, column_name, table_name,
             column_name, table_name, column_name
           );
  if (result != SQLITE_OK) {
    error_append(error, "Could not create geometry insert trigger: %s", sqlite3_errmsg(db));
    return result;
  }

  return SQLITE_OK;
}

/*
 * (geometry blob, geometry_type text, srid int, dimension text);
 * (geometry blob, geometry_type int, srid int);
 */
static void spl_geometry_constraints(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  const spatialdb_t *spatialdb;
  FUNCTION_WKB_ARG(wkb);
  FUNCTION_TEXT_ARG(expected_geometry_type_text);
  FUNCTION_INT_ARG(expected_geometry_type_int);
  FUNCTION_INT_ARG(expected_srid);
  FUNCTION_TEXT_ARG(expected_dimension_text);
  geom_header_t expected;

  FUNCTION_START_STATIC(context, 256);
  spatialdb = (const spatialdb_t *)sqlite3_user_data(context);

  if (nbArgs == 3) {
    FUNCTION_GET_INT_ARG(expected_geometry_type_int, 1);
    FUNCTION_GET_INT_ARG(expected_srid, 2);
    FUNCTION_GET_WKB_ARG_UNSAFE(context, spatialdb, wkb, 0);
    wkb_fill_geom_header((uint32_t)expected_geometry_type_int, &expected, FUNCTION_ERROR);
  } else {
    FUNCTION_GET_TEXT_ARG(context, expected_geometry_type_text, 1);
    FUNCTION_GET_INT_ARG(expected_srid, 2);
    FUNCTION_GET_TEXT_ARG(context, expected_dimension_text, 3);
    FUNCTION_GET_WKB_ARG_UNSAFE(context, spatialdb, wkb, 0);

    FUNCTION_RESULT = geom_type_from_string(expected_geometry_type_text, &expected.geom_type);
    if (FUNCTION_RESULT != SQLITE_OK) {
      error_append(FUNCTION_ERROR, "Invalid geometry type %s", expected_geometry_type_text);
      goto exit;
    }

    if (sqlite3_strnicmp(expected_dimension_text, "xy", 2) == 0) {
      expected.coord_size = 2;
      expected.coord_type = GEOM_XY;
    } else if (sqlite3_strnicmp(expected_dimension_text, "xyz", 3) == 0) {
      expected.coord_size = 3;
      expected.coord_type = GEOM_XYZ;
    } else if (sqlite3_strnicmp(expected_dimension_text, "xym", 3) == 0) {
      expected.coord_size = 3;
      expected.coord_type = GEOM_XYM;
    } else if (sqlite3_strnicmp(expected_dimension_text, "xyzm", 4) == 0) {
      expected.coord_size = 4;
      expected.coord_type = GEOM_XYZM;
    } else {
      error_append(FUNCTION_ERROR, "Unsupported geometry dimension: %s", expected_dimension_text);
      goto exit;
    }
  }

  if (geom_is_assignable(expected.geom_type, wkb.geom_type) == 0) {
    const char *expected_name;
    geom_type_name(expected.geom_type, &expected_name);
    const char *actual_name;
    geom_type_name(wkb.geom_type, &actual_name);
    error_append(FUNCTION_ERROR, "Geometry of type %s can not be written to column of type %s", actual_name, expected_name);
    goto exit;
  }

  if (FUNCTION_WKB_ARG_GEOM(wkb).srid != expected_srid) {
    error_append(FUNCTION_ERROR, "Geometry of with srid %d can not be written to column with srid %d", FUNCTION_WKB_ARG_GEOM(wkb).srid, expected_srid);
    goto exit;
  }

  if (wkb.coord_type != expected.coord_type) {
    const char *expected_coord_type;
    geom_coord_type_name(expected.coord_type, &expected_coord_type);
    const char *actual_coord_type;
    geom_coord_type_name(wkb.coord_type, &actual_coord_type);
    error_append(FUNCTION_ERROR, "%s geometry can not be written to %s column", actual_coord_type, expected_coord_type);
    goto exit;
  }

  sqlite3_result_int(context, 1);

  FUNCTION_END(context);
  FUNCTION_FREE_WKB_ARG(wkb);
  FUNCTION_FREE_TEXT_ARG(expected_geometry_type_text);
  FUNCTION_FREE_INT_ARG(expected_geometry_type_int);
  FUNCTION_FREE_INT_ARG(expected_srid);
  FUNCTION_FREE_TEXT_ARG(expected_dimension_text);
}

static int fill_envelope(binstream_t *stream, geom_envelope_t *envelope, errorstream_t *error) {
  return wkb_fill_envelope(stream, WKB_SPATIALITE, envelope, error);
}

static int read_geometry_header(binstream_t *stream, geom_header_t *header, errorstream_t *error) {
  return wkb_read_header(stream, WKB_SPATIALITE, header, error);
}

static int read_geometry(binstream_t *stream, geom_consumer_t const *consumer, errorstream_t *error) {
  return wkb_read_geometry(stream, WKB_SPATIALITE, consumer, error);
}

static int create_spatial_index(sqlite3 *db, const char *db_name, const char *table_name, const char *geometry_column_name, const char *id_column_name, errorstream_t *error) {
  int result = SQLITE_OK;
  char *index_table_name = NULL;
  int exists = 0;

  index_table_name = sqlite3_mprintf("idx_%s_%s", table_name, geometry_column_name);
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
  result = sql_exec_for_int(db, &geom_col_count, "SELECT count(*) FROM \"%w\".geometry_columns WHERE f_table_name LIKE %Q AND f_geometry_column LIKE %Q", db_name, table_name, geometry_column_name);
  if (result != SQLITE_OK) {
    error_append(error, "Could not check if column %s.%s.%s exists in %s.geometry_columns: %s", db_name, table_name, geometry_column_name, db_name, sqlite3_errmsg(db));
    goto exit;
  }

  if (geom_col_count == 0) {
    error_append(error, "Column %s.%s.%s is not registered in %s.geometry_columns", db_name, table_name, geometry_column_name, db_name);
    goto exit;
  }

  result = sql_exec(db, "UPDATE \"%w\".geometry_columns SET spatial_index_enabled = 1 WHERE f_table_name LIKE %Q AND f_geometry_column LIKE %Q and spatial_index_enabled = 0", db_name, table_name,
                    geometry_column_name);
  if (result != SQLITE_OK) {
    error_append(error, "Could not set spatial index enabled flag for column %s.%s.%s: %s", db_name, table_name, geometry_column_name, db_name, sqlite3_errmsg(db));
    goto exit;
  }

  if (sqlite3_changes(db) == 0) {
    // Spatial index already enabled; silent return
    goto exit;
  }

  result = sql_exec(db, "DROP TABLE IF EXISTS \"%w\".\"%w\"", db_name, index_table_name);
  if (result != SQLITE_OK) {
    error_append(error, "Could not drop old rtree table %s.%s: %s", db_name, index_table_name, sqlite3_errmsg(db));
    goto exit;
  }
  result = sql_exec(db, "DROP TRIGGER IF EXISTS \"%w\".\"gii_%w_%w\"", db_name, table_name, geometry_column_name);
  if (result != SQLITE_OK) {
    error_append(error, "Could not drop old rtree insert trigger %s.gii_%s_%s: %s", db_name, table_name, geometry_column_name, sqlite3_errmsg(db));
    goto exit;
  }
  result = sql_exec(db, "DROP TRIGGER IF EXISTS \"%w\".\"giu_%w_%w\"", db_name, table_name, geometry_column_name);
  if (result != SQLITE_OK) {
    error_append(error, "Could not drop old rtree update trigger %s.gii_%s_%s: %s", db_name, table_name, geometry_column_name, sqlite3_errmsg(db));
    goto exit;
  }
  result = sql_exec(db, "DROP TRIGGER IF EXISTS \"%w\".\"gid_%w_%w\"", db_name, table_name, geometry_column_name);
  if (result != SQLITE_OK) {
    error_append(error, "Could not drop old rtree delete trigger %s.gii_%s_%s: %s", db_name, table_name, geometry_column_name, sqlite3_errmsg(db));
    goto exit;
  }

  result = sql_exec(db, "CREATE VIRTUAL TABLE \"%w\".\"%w\" USING rtree(pkid, xmin, xmax, ymin, ymax)", db_name, index_table_name);
  if (result != SQLITE_OK) {
    error_append(error, "Could not create rtree table %s.%s: %s", db_name, index_table_name, sqlite3_errmsg(db));
    goto exit;
  }

  result = sql_exec(
             db,
             "CREATE TRIGGER \"%w\".\"gii_%w_%w\" AFTER INSERT ON \"%w\"\n"
             "BEGIN\n"
             "  SELECT RTreeAlign(\"%w\", NEW.\"%w\", NEW.\"%w\");\n"
             "END;",
             db_name, table_name, geometry_column_name, table_name,
             index_table_name, id_column_name, geometry_column_name
           );
  if (result != SQLITE_OK) {
    error_append(error, "Could not create rtree insert trigger: %s", sqlite3_errmsg(db));
    goto exit;
  }

  result = sql_exec(
             db,
             "CREATE TRIGGER \"%w\".\"giu_%w_%w\" AFTER UPDATE ON \"%w\"\n"
             "BEGIN\n"
             "  DELETE FROM \"%w\" WHERE pkid = OLD.\"%w\";\n"
             "  SELECT RTreeAlign(\"%w\", NEW.\"%w\", NEW.\"%w\");\n"
             "END;",
             db_name, table_name, geometry_column_name, table_name,
             index_table_name, id_column_name,
             index_table_name, id_column_name, geometry_column_name
           );
  if (result != SQLITE_OK) {
    error_append(error, "Could not create rtree update trigger: %s", sqlite3_errmsg(db));
    goto exit;
  }

  result = sql_exec(
             db,
             "CREATE TRIGGER \"%w\".\"gid_%w_%w\" AFTER DELETE ON \"%w\"\n"
             "BEGIN\n"
             "  DELETE FROM \"%w\" WHERE pkid = OLD.\"%w\";\n"
             "END;",
             db_name, table_name, geometry_column_name, table_name,
             index_table_name, id_column_name
           );
  if (result != SQLITE_OK) {
    error_append(error, "Could not create rtree delete trigger: %s", sqlite3_errmsg(db));
    goto exit;
  }

  result = sql_exec_all(
             db,
             "SELECT RTreeAlign(\"%w\", \"%w\", \"%w\") FROM \"%w\".\"%w\""
             "  WHERE \"%w\" NOTNULL AND NOT ST_IsEmpty(\"%w\")",
             index_table_name, id_column_name, geometry_column_name, db_name, table_name,
             geometry_column_name, geometry_column_name
           );
  if (result != SQLITE_OK) {
    error_append(error, "Could not populate rtree: %s", sqlite3_errmsg(db));
    goto exit;
  }

exit:
  sqlite3_free(index_table_name);
  return result;
}

/*
 * (indx_table_name text, \"%w\" int, geometry blob)
 */
static void spl_rtree_align(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  const spatialdb_t *spatialdb;
  FUNCTION_TEXT_ARG(index_table_name);
  FUNCTION_TEXT_ARG(row_id);
  FUNCTION_GEOM_ARG(geom);

  FUNCTION_START_STATIC(context, 256);
  spatialdb = (const spatialdb_t *)sqlite3_user_data(context);
  FUNCTION_GET_TEXT_ARG(context, index_table_name, 0);
  FUNCTION_GET_TEXT_ARG(context, row_id, 1);

  int delete_row = 0;
  if (sqlite3_value_type(args[2]) == SQLITE_NULL) {
    delete_row = 1;
  } else {
    FUNCTION_GET_GEOM_ARG_UNSAFE(context, spatialdb, geom, 2);
    delete_row = geom.empty;
  }

  if (delete_row) {
    FUNCTION_RESULT = sql_exec(
                        sqlite3_context_db_handle(context),
                        "DELETE FROM \"%w\" WHERE pkid = %s",
                        index_table_name, row_id
                      );
  } else {
    FUNCTION_RESULT = sql_exec(
                        sqlite3_context_db_handle(context),
                        "INSERT OR REPLACE INTO \"%w\" (pkid, xmin, ymin, xmax, ymax) VALUES (%s, %1.12f, %1.12f, %1.12f, %1.12f)",
                        index_table_name, row_id, geom.envelope.min_x, geom.envelope.min_y, geom.envelope.max_x, geom.envelope.max_y
                      );
  }

  if (FUNCTION_RESULT != SQLITE_OK) {
    error_append(FUNCTION_ERROR, sqlite3_errmsg(sqlite3_context_db_handle(context)));
  }

  FUNCTION_END(context);
  FUNCTION_FREE_TEXT_ARG(index_table_name);
  FUNCTION_FREE_TEXT_ARG(row_id);
  FUNCTION_FREE_GEOM_ARG(geom);
}

static void spatialite_init(sqlite3 *db, const spatialdb_t *spatialDb, errorstream_t *error) {
  sql_create_function(db, "GeometryConstraints", spl_geometry_constraints, 3, SQL_DETERMINISTIC, (void *)spatialDb, NULL, error);
  sql_create_function(db, "GeometryConstraints", spl_geometry_constraints, 4, SQL_DETERMINISTIC, (void *)spatialDb, NULL, error);
  sql_create_function(db, "RTreeAlign", spl_rtree_align, 3, 0, (void *)spatialDb, NULL, error);
}

static const spatialdb_t SPATIALITE2 = {
  "Spatialite2",
  spatialite_init,
  spl2_init,
  spl2_check,
  write_blob_header,
  read_blob_header,
  spl3_writer_init,
  spb_writer_init,
  spb_writer_destroy,
  spl2_add_geometry_column,
  NULL,
  create_spatial_index,
  fill_envelope,
  read_geometry_header,
  read_geometry
};

static const spatialdb_t SPATIALITE3 = {
  "Spatialite3",
  spatialite_init,
  spl3_init,
  spl3_check,
  write_blob_header,
  read_blob_header,
  spl3_writer_init,
  spb_writer_init,
  spb_writer_destroy,
  spl3_add_geometry_column,
  NULL,
  create_spatial_index,
  fill_envelope,
  read_geometry_header,
  read_geometry
};

static const spatialdb_t SPATIALITE4 = {
  "Spatialite4",
  spatialite_init,
  spl4_init,
  spl4_check,
  write_blob_header,
  read_blob_header,
  spl4_writer_init,
  spb_writer_init,
  spb_writer_destroy,
  spl4_add_geometry_column,
  NULL,
  create_spatial_index,
  fill_envelope,
  read_geometry_header,
  read_geometry
};

const spatialdb_t *spatialdb_spatialite2_schema() {
  return &SPATIALITE2;
}

const spatialdb_t *spatialdb_spatialite3_schema() {
  return &SPATIALITE3;
}

const spatialdb_t *spatialdb_spatialite4_schema() {
  return &SPATIALITE4;
}
