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
#include <stdint.h>
#include "binstream.h"
#include "check.h"
#include "geomio.h"
#include "gpkg.h"
#include "gpb.h"
#include "sql.h"
#include "sqlite.h"
#include "tables.h"
#include "wkb.h"
#include "wkt.h"

#ifdef GPKG_HAVE_CONFIG_H
#include "config.h"
#endif

SQLITE_EXTENSION_INIT1

#define TEXT_FUNC_START(context, args, text, error) \
  error_t error;\
  char error_buffer[256];\
  char *text = (char *) sqlite3_value_text(args[0]);\
  size_t length = (size_t) sqlite3_value_bytes(args[0]);\
\
  if (text == NULL || length == 0) {\
    sqlite3_result_null(context);\
    return;\
  }\
\
  if (error_init_fixed(&error, error_buffer, 256) != SQLITE_OK) {\
    sqlite3_result_error(context, "Could not init error buffer", -1);\
    goto exit;\
  }

#define TEXT_FUNC_END \
exit:\
  error_destroy(&error);

#define BLOB_FUNC_START(context, args, stream, error) \
  error_t error;\
  char error_buffer[256];\
  binstream_t stream;\
\
  uint8_t *blob = (uint8_t *) sqlite3_value_blob(args[0]);\
  size_t length = (size_t) sqlite3_value_bytes(args[0]);\
  if (blob == NULL || length == 0) {\
    sqlite3_result_null(context);\
    return;\
  }\
\
  if (error_init_fixed(&error, error_buffer, 256) != SQLITE_OK) {\
    sqlite3_result_error(context, "Could not init error buffer", -1);\
    goto exit;\
  }\
\
  binstream_init(&stream, blob, length);

#define BLOB_FUNC_END \
exit:\
  binstream_destroy(&stream);\
  error_destroy(&error);

#define GPB_FUNC_START(context, args, gpb, stream, error) \
  gpb_header_t gpb;\
  BLOB_FUNC_START(context, args, stream, error) \
  if (gpb_read_header(&stream, &gpb, &error) != SQLITE_OK) {\
    sqlite3_result_error(context, error_count(&error) > 0 ? error_message(&error) : "Invalid GPB header", -1);\
    goto exit;\
  }

#define GPB_FUNC_END BLOB_FUNC_END

#define WKB_FUNC_START(context, args, gpb, wkb, stream, error) \
  GPB_FUNC_START(context, args, gpb, stream, error) \
  geom_header_t wkb;\
  if (wkb_read_header(&stream, &wkb, &error) != SQLITE_OK) {\
    sqlite3_result_error(context, error_count(&error) > 0 ? error_message(&error) : "Invalid GPB header", -1);\
    goto exit;\
  }

#define WKB_FUNC_END GPB_FUNC_END

#define ST_MIN_MAX(name, check, field) static void ST_##name(sqlite3_context *context, int nbArgs, sqlite3_value **args) { \
    GPB_FUNC_START(context, args, gpb, stream, error) \
 \
    if (gpb.envelope.check == 0) { \
        if (wkb_fill_envelope(&stream, &gpb.envelope, &error) != SQLITE_OK) { \
            sqlite3_result_error(context, error_count(&error) > 0 ? error_message(&error) : "Invalid GPB header", -1); \
            return; \
        } \
    } \
\
    if (gpb.envelope.check) { \
        sqlite3_result_double(context, gpb.envelope.field); \
    } else { \
        sqlite3_result_null(context); \
    } \
    GPB_FUNC_END \
}

ST_MIN_MAX(MinX, has_env_x, min_x)
ST_MIN_MAX(MaxX, has_env_x, max_x)
ST_MIN_MAX(MinY, has_env_y, min_y)
ST_MIN_MAX(MaxY, has_env_y, max_y)
ST_MIN_MAX(MinZ, has_env_z, min_z)
ST_MIN_MAX(MaxZ, has_env_z, max_z)
ST_MIN_MAX(MinM, has_env_m, min_m)
ST_MIN_MAX(MaxM, has_env_m, max_m)

static void ST_SRID(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
    GPB_FUNC_START(context, args, gpb, stream, error)

    if (nbArgs == 1) {
        sqlite3_result_int(context, gpb.srid);
    } else {
        gpb.srid = sqlite3_value_int(args[1]);
        if (binstream_seek(&stream, 0) != SQLITE_OK) {
            sqlite3_result_error(context, "Error writing GPB header", -1);
            goto exit;
        }
        if (gpb_write_header(&stream, &gpb, &error) != SQLITE_OK) {
            sqlite3_result_error(context, error_count(&error) > 0 ? error_message(&error) : "Error writing GPB header", -1);
            goto exit;
        }
        binstream_seek(&stream, 0);
        sqlite3_result_blob(context, binstream_data(&stream), (int) binstream_available(&stream), SQLITE_TRANSIENT);
    }

    GPB_FUNC_END
}

static void ST_IsEmpty(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
    GPB_FUNC_START(context, args, gpb, stream, error)

    sqlite3_result_int(context, gpb.empty);

    GPB_FUNC_END
}

static void ST_IsMeasured(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
    WKB_FUNC_START(context, args, gpb, wkb, stream, error)

    sqlite3_result_int(context, wkb.coord_type == GEOM_XYM || wkb.coord_type == GEOM_XYZM);

    WKB_FUNC_END
}

static void ST_Is3d(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
    WKB_FUNC_START(context, args, gpb, wkb, stream, error)

    sqlite3_result_int(context, wkb.coord_type == GEOM_XYZ || wkb.coord_type == GEOM_XYZM);

    WKB_FUNC_END
}

static void ST_IsValid(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
    geom_consumer_t consumer;
    GPB_FUNC_START(context, args, gpb, stream, error)

    geom_consumer_init(&consumer, NULL, NULL, NULL, NULL, NULL);
    if (wkb_read_geometry(&stream, &consumer, NULL) != SQLITE_OK) {
        sqlite3_result_int(context, 0);
        goto exit;
    }

    sqlite3_result_int(context, 1);

    GPB_FUNC_END
}

static void ST_CoordDim(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
    WKB_FUNC_START(context, args, gpb, wkb, stream, error)
    sqlite3_result_int(context, geom_coord_dim(&wkb));
    WKB_FUNC_END
}

static void ST_GeometryType(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
    WKB_FUNC_START(context, args, gpb, wkb, stream, error)
    const char *type_name = geom_type_name(&wkb);
    if (type_name) {
        sqlite3_result_text(context, type_name, -1, SQLITE_STATIC);
    } else {
        sqlite3_result_error(context, "Unknown geometry type", -1);
    }
    WKB_FUNC_END
}

static void ST_AsBinary(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
    GPB_FUNC_START(context, args, gpb, stream, error)
    sqlite3_result_blob(context, binstream_data(&stream), (int) binstream_available(&stream), SQLITE_TRANSIENT);
    GPB_FUNC_END
}

static void ST_GeomFromWKB(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
    BLOB_FUNC_START(context, args, stream, error)

    int32_t srid = -1;
    if (nbArgs == 2) {
        srid = (int32_t) sqlite3_value_int(args[1]);
    }

    gpb_writer_t writer;
    gpb_writer_init(&writer, srid);

    int result = wkb_read_geometry(&stream, gpb_writer_geom_consumer(&writer), &error);

    if (result != SQLITE_OK) {
        sqlite3_result_error(context, error_count(&error) > 0 ? error_message(&error) : "Could not parse WKB", -1);
    } else {
        sqlite3_result_blob(context, gpb_writer_getgpb(&writer), (int) gpb_writer_length(&writer), SQLITE_TRANSIENT);
    }

    gpb_writer_destroy(&writer);

    BLOB_FUNC_END
}

static void ST_AsText(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
    GPB_FUNC_START(context, args, gpb, stream, error)

    wkt_writer_t writer;
    wkt_writer_init(&writer);
    int result = wkb_read_geometry(&stream, wkt_writer_geom_consumer(&writer), &error);

    if (result != SQLITE_OK) {
        sqlite3_result_error(context, error_count(&error) > 0 ? error_message(&error) : "Could not parse WKB", -1);
    } else {
        sqlite3_result_text(context, wkt_writer_getwkt(&writer), (int) wkt_writer_length(&writer), SQLITE_TRANSIENT);
    }
    wkt_writer_destroy(&writer);

    GPB_FUNC_END
}

static void ST_GeomFromText(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
    TEXT_FUNC_START(context, args, text, error)

    int32_t srid = -1;
    if (nbArgs == 2) {
        srid = (int32_t) sqlite3_value_int(args[1]);
    }

    gpb_writer_t writer;
    gpb_writer_init(&writer, srid);
    int result = wkt_read_geometry(text, length, gpb_writer_geom_consumer(&writer), &error);

    if (result != SQLITE_OK) {
        sqlite3_result_error(context, error_count(&error) > 0 ? error_message(&error) : "Could not parse WKT", -1);
    } else {
        sqlite3_result_blob(context, gpb_writer_getgpb(&writer), (int) gpb_writer_length(&writer), SQLITE_TRANSIENT);
    }

    gpb_writer_destroy(&writer);

    TEXT_FUNC_END
}

static void ST_WKBFromText(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
    TEXT_FUNC_START(context, args, text, error)

    wkb_writer_t writer;
    wkb_writer_init(&writer);
    int result = wkt_read_geometry(text, length, wkb_writer_geom_consumer(&writer), &error);
    if (result != SQLITE_OK) {
        sqlite3_result_error(context, error_count(&error) > 0 ? error_message(&error) : "Could not parse WKT", -1);
    } else {
        sqlite3_result_blob(context, wkb_writer_getwkb(&writer), (int) wkb_writer_length(&writer), SQLITE_TRANSIENT);
    }
    wkb_writer_destroy(&writer);

    TEXT_FUNC_END
}

#define FUNCTION_RESULT result
#define FUNCTION_DB_HANDLE db_handle
#define FUNCTION_ERROR error
#define FUNCTION_ERROR_PTR &FUNCTION_ERROR

#define FUNCTION_START(context) \
    int FUNCTION_RESULT = SQLITE_OK;\
    int arg_counter = 0;\
    error_t FUNCTION_ERROR;\
    FUNCTION_RESULT = error_init(FUNCTION_ERROR_PTR);\
    if (FUNCTION_RESULT != SQLITE_OK) {\
        goto exit;\
    }\
    sqlite3 *FUNCTION_DB_HANDLE = sqlite3_context_db_handle(context);

#define FUNCTION_END(context) \
  exit:\
    if (FUNCTION_RESULT == SQLITE_OK) {\
        if (error_count(FUNCTION_ERROR_PTR) > 0) {\
            sqlite3_result_error(context, error_message(FUNCTION_ERROR_PTR), -1);\
        } else {\
            sqlite3_result_null(context);\
        }\
    } else {\
        sqlite3_result_error(context, error_message(FUNCTION_ERROR_PTR), -1);\
    }\
    error_destroy(FUNCTION_ERROR_PTR);

#define FUNCTION_START_TRANSACTION(name) \
    char *name##_transaction = #name;\
    FUNCTION_RESULT = sql_begin(FUNCTION_DB_HANDLE, name##_transaction);\
    if (FUNCTION_RESULT != SQLITE_OK) {\
        goto exit;\
    }
#define FUNCTION_END_TRANSACTION(name) \
    if (FUNCTION_RESULT == SQLITE_OK && error_count(FUNCTION_ERROR_PTR) == 0) {\
        FUNCTION_RESULT = sql_commit(FUNCTION_DB_HANDLE, name##_transaction);\
    } else {\
        sql_rollback(FUNCTION_DB_HANDLE, name##_transaction);\
    }                               
                           
#define FUNCTION_TEXT_ARG(arg) \
    char* arg = NULL;\
    int free_##arg = 0;
#define FUNCTION_GET_TEXT_ARG(context, arg) \
    arg = sqlite3_mprintf("%s", sqlite3_value_text(args[arg_counter++]));\
    free_##arg = 1;\
    if (arg == NULL) {\
        sqlite3_result_error_code(context, SQLITE_NOMEM);\
        goto exit;\
    }
#define FUNCTION_SET_TEXT_ARG(arg, val) \
    arg = val;\
    free_##arg = 0;
#define FUNCTION_FREE_TEXT_ARG(arg) \
    if (free_##arg != 0) {\
        sqlite3_free(arg);\
        arg = NULL;\
    }

#define FUNCTION_INT_ARG(arg) int arg = 0;
#define FUNCTION_GET_INT_ARG(arg) arg = sqlite3_value_int(args[arg_counter++]);
#define FUNCTION_SET_INT_ARG(arg, val) arg = val;
#define FUNCTION_FREE_INT_ARG(arg)

static void CheckGpkg(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
    FUNCTION_TEXT_ARG(db_name)
    FUNCTION_START(context)

    if (nbArgs == 0) {
        FUNCTION_SET_TEXT_ARG(db_name,"main");
    } else {
        FUNCTION_GET_TEXT_ARG(context, db_name);
    }

    FUNCTION_RESULT = check_gpkg(FUNCTION_DB_HANDLE, db_name, FUNCTION_ERROR_PTR);

    FUNCTION_END(context)
    FUNCTION_FREE_TEXT_ARG(db_name)
}

static int InitGpkg_(sqlite3 *db, char *db_name, error_t *error) {
    int result = SQLITE_OK;
    const table_info_t * const *table = tables;

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

static void InitGpkg(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
    FUNCTION_TEXT_ARG(db_name)
    FUNCTION_START(context)

    if (nbArgs == 0) {
        FUNCTION_SET_TEXT_ARG(db_name,"main");
    } else {
        FUNCTION_GET_TEXT_ARG(context, db_name);
    }

    FUNCTION_START_TRANSACTION(__initgpkg);
    FUNCTION_RESULT = InitGpkg_(FUNCTION_DB_HANDLE, db_name, FUNCTION_ERROR_PTR);
    FUNCTION_END_TRANSACTION(__initgpkg);

    FUNCTION_END(context)
    FUNCTION_FREE_TEXT_ARG(db_name)
}

static int AddGeometryColumn_(sqlite3 *db, char *db_name, char *table_name, char *column_name, char *geom_type, int srs_id, int z, int m, error_t *error) {
    int result;

    result = geom_type_from_string(geom_type, NULL);
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

    // Check if required meta tables exist
    result = sql_check_table(db, db_name, &gpkg_spatial_ref_sys, error);
    if (result != SQLITE_OK) {
        return result;
    }
    result = sql_check_table(db, db_name, &gpkg_geometry_columns, error);
    if (result != SQLITE_OK) {
        return result;
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

    result = sql_exec(db, "ALTER TABLE \"%w\".\"%w\" ADD COLUMN \"%w\" %s", db_name, table_name, column_name, geom_type);
    if (result != SQLITE_OK) {
        error_append(error, sqlite3_errmsg(db));
        return result;
    }

    result = sql_exec(db, "INSERT INTO \"%w\".\"%w\" (table_name, column_name, geometry_type, srs_id, z, m) VALUES (%Q, %Q, %Q, %d, %d, %d)", db_name, "gpkg_geometry_columns", table_name, column_name, geom_type, srs_id, z, m);
    if (result != SQLITE_OK) {
        error_append(error, sqlite3_errmsg(db));
        return result;
    }

    return SQLITE_OK;
}

static void AddGeometryColumn(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
    FUNCTION_TEXT_ARG(db_name)
    FUNCTION_TEXT_ARG(table_name)
    FUNCTION_TEXT_ARG(column_name)
    FUNCTION_TEXT_ARG(geometry_type)
    FUNCTION_INT_ARG(srs_id)
    FUNCTION_INT_ARG(z)
    FUNCTION_INT_ARG(m)
    FUNCTION_START(context)

    if (nbArgs == 7) {
        FUNCTION_GET_TEXT_ARG(context, db_name)
    } else {
        FUNCTION_SET_TEXT_ARG(db_name,"main")
    }
    FUNCTION_GET_TEXT_ARG(context, table_name)
    FUNCTION_GET_TEXT_ARG(context, column_name)
    FUNCTION_GET_TEXT_ARG(context, geometry_type)
    FUNCTION_GET_INT_ARG(srs_id)
    FUNCTION_GET_INT_ARG(z)
    FUNCTION_GET_INT_ARG(m)


    FUNCTION_START_TRANSACTION(__add_geom_col)

    FUNCTION_RESULT = InitGpkg_(FUNCTION_DB_HANDLE, db_name, FUNCTION_ERROR_PTR);

    if (FUNCTION_RESULT == SQLITE_OK) {
      FUNCTION_RESULT = AddGeometryColumn_(FUNCTION_DB_HANDLE, db_name, table_name, column_name, geometry_type, srs_id, z, m, FUNCTION_ERROR_PTR);
    }
    FUNCTION_END_TRANSACTION(__add_geom_col)

    FUNCTION_END(context)
    FUNCTION_FREE_TEXT_ARG(db_name)
    FUNCTION_FREE_TEXT_ARG(table_name)
    FUNCTION_FREE_TEXT_ARG(column_name)
    FUNCTION_FREE_TEXT_ARG(geometry_type)
    FUNCTION_FREE_INT_ARG(srid)
    FUNCTION_FREE_INT_ARG(z)
    FUNCTION_FREE_INT_ARG(m)
}

static int CreateTilesTable_(sqlite3 *db, char *db_name, char *table_name, error_t *error) {
    int result = SQLITE_OK;

    // Check if the target table exists
    int exists = 0;
    result= sql_check_table_exists(db, db_name, table_name, &exists);
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
        tiles_table_columns,
        NULL, 0
    };

    // Check if required meta tables exist
    result = sql_init_table(db, db_name, &tile_table_info, error);
    if (result != SQLITE_OK) {
        return result;
    }

    return SQLITE_OK;
}

static void CreateTilesTable(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
    FUNCTION_TEXT_ARG(db_name)
    FUNCTION_TEXT_ARG(table_name)
    FUNCTION_START(context)

    if (nbArgs == 2) {
        FUNCTION_GET_TEXT_ARG(context, db_name)
    } else {
        FUNCTION_SET_TEXT_ARG(db_name, "main")
    }
    FUNCTION_GET_TEXT_ARG(context, table_name)

    FUNCTION_START_TRANSACTION(__create_tiles_table)
    FUNCTION_RESULT = InitGpkg_(FUNCTION_DB_HANDLE, db_name, FUNCTION_ERROR_PTR);
    if (FUNCTION_RESULT == SQLITE_OK) {
        FUNCTION_RESULT = CreateTilesTable_(FUNCTION_DB_HANDLE, db_name, table_name, FUNCTION_ERROR_PTR);
    }
    FUNCTION_END_TRANSACTION(__create_tiles_table)

    FUNCTION_END(context)
    FUNCTION_FREE_TEXT_ARG(db_name)
    FUNCTION_FREE_TEXT_ARG(table_name)
}

static int CreateSpatialIndex_(sqlite3 *db, char *db_name, char *table_name, char *column_name, error_t *error) {
    int result = SQLITE_OK;
    char* index_table_name = NULL;
    int exists = 0;

    index_table_name = sqlite3_mprintf("rtree_%s_%s", table_name, column_name);
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
    result = sql_exec_for_int(db, &geom_col_count, "SELECT count(*) FROM \"%w\".gpkg_geometry_columns WHERE table_name LIKE %Q AND column_name LIKE %Q", db_name, table_name, column_name);
    if (result != SQLITE_OK) {
        error_append(error, "Could not check if column %s.%s.%s exists in %s.gpkg_geometry_columns: %s", db_name, table_name, column_name, db_name, sqlite3_errmsg(db));
        goto exit;
    }

    if (geom_col_count == 0) {
        error_append(error, "Column %s.%s.%s is not registered in %s.gpkg_geometry_columns", db_name, table_name, column_name, db_name);
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
        "    NEW.rowid,\n"
        "    ST_MinX(NEW.\"%w\"), ST_MaxX(NEW.\"%w\"),\n"
        "    ST_MinY(NEW.\"%w\"), ST_MaxY(NEW.\"%w\")\n"
        "  );\n"
        "END;",
        db_name, table_name, column_name, table_name,
        column_name, column_name,
        index_table_name,
        column_name, column_name,
        column_name, column_name
    );
    if (result != SQLITE_OK) {
        error_append(error, "Could not create rtree insert trigger: %s", sqlite3_errmsg(db));
        goto exit;
    }

    result = sql_exec(
        db,
        "CREATE TRIGGER \"%w\".\"rtree_%w_%w_update1\" AFTER UPDATE OF \"%w\" ON \"%w\"\n"
        "    WHEN OLD.rowid = NEW.rowid AND\n"
        "         (NEW.\"%w\" NOTNULL AND NOT ST_IsEmpty(NEW.\"%w\"))\n"
        "BEGIN\n"
        "  INSERT OR REPLACE INTO \"%w\" VALUES (\n"
        "    NEW.rowid,\n"
        "    ST_MinX(NEW.\"%w\"), ST_MaxX(NEW.\"%w\"),\n"
        "    ST_MinY(NEW.\"%w\"), ST_MaxY(NEW.\"%w\")\n"
        "  );\n"
        "END;",
        db_name, table_name, column_name, column_name, table_name,
        column_name, column_name,
        index_table_name,
        column_name, column_name,
        column_name, column_name
    );
    if (result != SQLITE_OK) {
        error_append(error, "Could not create rtree update trigger 1: %s", sqlite3_errmsg(db));
        goto exit;
    }

    result = sql_exec(
        db,
        "CREATE TRIGGER \"%w\".\"rtree_%w_%w_update2\" AFTER UPDATE OF \"%w\" ON \"%w\"\n"
        "    WHEN OLD.rowid = NEW.rowid AND\n"
        "         (NEW.\"%w\" ISNULL OR ST_IsEmpty(NEW.\"%w\"))\n"
        "BEGIN\n"
        "  DELETE FROM \"%w\" WHERE id = OLD.rowid;\n"
        "END;",
        db_name, table_name, column_name, column_name, table_name,
        column_name, column_name,
        index_table_name
    );
    if (result != SQLITE_OK) {
        error_append(error, "Could not create rtree update trigger 2: %s", sqlite3_errmsg(db));
        goto exit;
    }

    result = sql_exec(
        db,
        "CREATE TRIGGER \"%w\".\"rtree_%w_%w_update3\" AFTER UPDATE ON \"%w\"\n"
        "    WHEN OLD.rowid != NEW.rowid AND\n"
        "         (NEW.\"%w\" NOTNULL AND NOT ST_IsEmpty(NEW.\"%w\"))\n"
        "BEGIN\n"
        "  DELETE FROM \"%w\" WHERE id = OLD.rowid;\n"
        "  INSERT OR REPLACE INTO \"%w\" VALUES (\n"
        "    NEW.rowid,\n"
        "    ST_MinX(NEW.\"%w\"), ST_MaxX(NEW.\"%w\"),\n"
        "    ST_MinY(NEW.\"%w\"), ST_MaxY(NEW.\"%w\")\n"
        "  );\n"
        "END;",
        db_name, table_name, column_name, table_name,
        column_name, column_name,
        index_table_name,
        index_table_name,
        column_name, column_name,
        column_name, column_name
    );
    if (result != SQLITE_OK) {
        error_append(error, "Could not create rtree update trigger 3: %s", sqlite3_errmsg(db));
        goto exit;
    }

    result = sql_exec(
        db,
        "CREATE TRIGGER \"%w\".\"rtree_%w_%w_update4\" AFTER UPDATE ON \"%w\"\n"
        "    WHEN OLD.rowid != NEW.rowid AND\n"
        "         (NEW.\"%w\" ISNULL OR ST_IsEmpty(NEW.\"%w\"))\n"
        "BEGIN\n"
        "  DELETE FROM \"%w\" WHERE id IN (OLD.rowid, NEW.rowid);\n"
        "END;",
        db_name, table_name, column_name, table_name,
        column_name, column_name,
        index_table_name
    );
    if (result != SQLITE_OK) {
        error_append(error, "Could not create rtree update trigger 4: %s", sqlite3_errmsg(db));
        goto exit;
    }

    result = sql_exec(
        db,
        "CREATE TRIGGER \"%w\".\"rtree_%w_%w_delete\" AFTER DELETE ON \"%w\"\n"
        "BEGIN\n"
        "  DELETE FROM \"%w\" WHERE id = OLD.rowid;\n"
        "END;",
        db_name, table_name, column_name, table_name, column_name, column_name,
        index_table_name
    );
    if (result != SQLITE_OK) {
        error_append(error, "Could not create rtree delete trigger: %s", sqlite3_errmsg(db));
        goto exit;
    }

    result = sql_exec(
        db,
        "INSERT OR REPLACE INTO \"%w\".\"%w\" (id, minx, maxx, miny, maxy) "
        "  SELECT rowid, ST_MinX(\"%w\"), ST_MaxX(\"%w\"), ST_MinY(\"%w\"), ST_MaxY(\"%w\") FROM \"%w\".\"%w\""
        "  WHERE \"%w\" NOTNULL AND NOT ST_IsEmpty(\"%w\")",
        db_name, index_table_name,
        column_name, column_name, column_name, column_name, db_name, table_name,
        column_name, column_name
    );
    if (result != SQLITE_OK) {
        error_append(error, "Could not populate rtree: %s", sqlite3_errmsg(db));
        goto exit;
    }

    result = sql_exec(
        db,
        "INSERT OR REPLACE INTO \"%w\".\"gpkg_extensions\" (table_name, column_name, extension_name) VALUES (\"%w\", \"%w\", \"%w\")",
        db_name, table_name, column_name, "gpkg_rtree_index"
    );
    if (result != SQLITE_OK) {
        error_append(error, "Could not register rtree usage in gpkg_extensions: %s", sqlite3_errmsg(db));
        goto exit;
    }

  exit:
    sqlite3_free(index_table_name);
    return result;
}

static void CreateSpatialIndex(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
    FUNCTION_TEXT_ARG(db_name)
    FUNCTION_TEXT_ARG(table_name)
    FUNCTION_TEXT_ARG(column_name)
    FUNCTION_START(context)

    if (nbArgs == 3) {
        FUNCTION_GET_TEXT_ARG(context, db_name)
    } else {
        FUNCTION_SET_TEXT_ARG(db_name, "main")
    }
    FUNCTION_GET_TEXT_ARG(context, table_name)
    FUNCTION_GET_TEXT_ARG(context, column_name)

    FUNCTION_START_TRANSACTION(__create_spatial_index)
    FUNCTION_RESULT = InitGpkg_(FUNCTION_DB_HANDLE, db_name, FUNCTION_ERROR_PTR);
    if (FUNCTION_RESULT == SQLITE_OK) {
        FUNCTION_RESULT = CreateSpatialIndex_(FUNCTION_DB_HANDLE, db_name, table_name, column_name, FUNCTION_ERROR_PTR);
    }
    FUNCTION_END_TRANSACTION(__create_spatial_index)

    FUNCTION_END(context)
    FUNCTION_FREE_TEXT_ARG(db_name)
    FUNCTION_FREE_TEXT_ARG(table_name)
}

#ifdef __cplusplus
extern "C" {
#endif

GPKG_EXPORT const char * GPKG_CALL gpkg_libversion(void) {
    return LIBGPKG_VERSION;
}

#define REGISTER_FUNC(name, function, args) if (sqlite3_create_function_v2( db, #name, args, SQLITE_UTF8, NULL, function, NULL, NULL, NULL ) != SQLITE_OK) return SQLITE_ERROR;
#define FUNC(name, args) REGISTER_FUNC(name, name, args)
#define ST_FUNC(name, args) REGISTER_FUNC(name, ST_##name, args) REGISTER_FUNC(ST_##name, ST_##name, args)
#define ST_ALIAS(name, function, args) REGISTER_FUNC(name, ST_##function, args) REGISTER_FUNC(ST_##name, ST_##function, args)

GPKG_EXPORT int GPKG_CALL sqlite3_gpkg_init(sqlite3 *db, const char **pzErrMsg, const sqlite3_api_routines *pThunk) {
    SQLITE_EXTENSION_INIT2(pThunk)

    if ( sqlite3_libversion_number() < 3007000 ) {
        if (pzErrMsg) {
            *pzErrMsg = sqlite3_mprintf( "libgpkg requires SQLite 3.7.0 or higher; detected %s", sqlite3_libversion() );
        }
        return SQLITE_ERROR;
    }

    ST_FUNC( MinX, 1 );
    ST_FUNC( MaxX, 1 );
    ST_FUNC( MinY, 1 );
    ST_FUNC( MaxY, 1 );
    ST_FUNC( MinZ, 1 );
    ST_FUNC( MaxZ, 1 );
    ST_FUNC( MinM, 1 );
    ST_FUNC( MaxM, 1 );
    ST_FUNC( SRID, 1 );
    ST_FUNC( SRID, 2 );
    ST_FUNC( Is3d, 1 );
    ST_FUNC( IsEmpty, 1 );
    ST_FUNC( IsMeasured, 1 );
    ST_FUNC( IsValid, 1 );
    ST_FUNC( CoordDim, 1 );
    ST_FUNC( GeometryType, 1 );
    ST_FUNC( AsBinary, 1 );
    ST_FUNC( GeomFromWKB, 1 );
    ST_ALIAS( WKBToSQL, GeomFromWKB, 1 );
    ST_FUNC( GeomFromWKB, 2 );
    ST_FUNC( AsText, 1 );
    ST_FUNC( GeomFromText, 1 );
    ST_FUNC( WKBFromText, 1 );
    ST_ALIAS( WKTToSQL, GeomFromText, 1 );
    ST_FUNC( GeomFromText, 2 );
    FUNC( CheckGpkg, 0 );
    FUNC( CheckGpkg, 1 );
    FUNC( InitGpkg, 0 );
    FUNC( InitGpkg, 1 );
    FUNC( AddGeometryColumn, 6 );
    FUNC( AddGeometryColumn, 7 );
    FUNC( CreateTilesTable, 1 );
    FUNC( CreateTilesTable, 2 );
    FUNC( CreateSpatialIndex, 2 );
    FUNC( CreateSpatialIndex, 3 );

    return SQLITE_OK;
}

#ifdef __cplusplus
}
#endif
