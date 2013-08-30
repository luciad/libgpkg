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
#include <stdint.h>
#include <stdio.h>
#include "binstream.h"
#include "blobio.h"
#include "check.h"
#include "geomio.h"
#include "sql.h"
#include "sqlite.h"
#include "spatialdb.h"
#include "wkb.h"
#include "wkt.h"

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

#define GEOMBLOB_FUNC_START(spatialdb, context, args, geomblob, stream, error) \
  const spatialdb_t *spatialdb = (const spatialdb_t *)sqlite3_user_data(context); \
  geom_blob_header_t geomblob;\
  BLOB_FUNC_START(context, args, stream, error) \
  if (spatialdb->read_blob_header(&stream, &geomblob, &error) != SQLITE_OK) {\
    sqlite3_result_error(context, error_count(&error) > 0 ? error_message(&error) : "Invalid geometry blob header", -1);\
    goto exit;\
  }

#define GEOMBLOB_FUNC_END BLOB_FUNC_END

#define WKB_FUNC_START(spatialdb, context, args, geomblob, wkb, stream, error) \
  GEOMBLOB_FUNC_START(spatialdb, context, args, geomblob, stream, error) \
  geom_header_t wkb;\
  if (spatialdb->read_geometry_header(&stream, &wkb, &error) != SQLITE_OK) {\
    sqlite3_result_error(context, error_count(&error) > 0 ? error_message(&error) : "Invalid geometry blob header", -1);\
    goto exit;\
  }

#define WKB_FUNC_END GEOMBLOB_FUNC_END

#define ST_MIN_MAX(name, check, field) static void ST_##name(sqlite3_context *context, int nbArgs, sqlite3_value **args) { \
    GEOMBLOB_FUNC_START(spatialdb, context, args, geomblob, stream, error) \
 \
    if (geomblob.envelope.check == 0) { \
        if (spatialdb->fill_envelope(&stream, &geomblob.envelope, &error) != SQLITE_OK) { \
            sqlite3_result_error(context, error_count(&error) > 0 ? error_message(&error) : "Invalid geometry blob header", -1); \
            return; \
        } \
    } \
\
    if (geomblob.envelope.check) { \
        sqlite3_result_double(context, geomblob.envelope.field); \
    } else { \
        sqlite3_result_null(context); \
    } \
    GEOMBLOB_FUNC_END \
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
  GEOMBLOB_FUNC_START(spatialdb, context, args, geomblob, stream, error)

  if (nbArgs == 1) {
    sqlite3_result_int(context, geomblob.srid);
  } else {
    geomblob.srid = sqlite3_value_int(args[1]);
    if (binstream_seek(&stream, 0) != SQLITE_OK) {
      sqlite3_result_error(context, "Error writing geometry blob header", -1);
      goto exit;
    }
    if (spatialdb->write_blob_header(&stream, &geomblob, &error) != SQLITE_OK) {
      sqlite3_result_error(context, error_count(&error) > 0 ? error_message(&error) : "Error writing geometry blob header", -1);
      goto exit;
    }
    binstream_seek(&stream, 0);
    sqlite3_result_blob(context, binstream_data(&stream), (int) binstream_available(&stream), SQLITE_TRANSIENT);
  }

  GEOMBLOB_FUNC_END
}

static void ST_IsEmpty(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  GEOMBLOB_FUNC_START(spatialdb, context, args, geomblob, stream, error)

  sqlite3_result_int(context, geomblob.empty);

  GEOMBLOB_FUNC_END
}

static void ST_IsMeasured(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  WKB_FUNC_START(spatialdb, context, args, geomblob, wkb, stream, error)

  sqlite3_result_int(context, wkb.coord_type == GEOM_XYM || wkb.coord_type == GEOM_XYZM);

  WKB_FUNC_END
}

static void ST_Is3d(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  WKB_FUNC_START(spatialdb, context, args, geomblob, wkb, stream, error)

  sqlite3_result_int(context, wkb.coord_type == GEOM_XYZ || wkb.coord_type == GEOM_XYZM);

  WKB_FUNC_END
}

static void ST_IsValid(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  GEOMBLOB_FUNC_START(spatialdb, context, args, geomblob, stream, error)

  geom_consumer_t consumer;
  geom_consumer_init(&consumer, NULL, NULL, NULL, NULL, NULL);
  if (spatialdb->read_geometry(&stream, &consumer, NULL) != SQLITE_OK) {
    sqlite3_result_int(context, 0);
    goto exit;
  }

  sqlite3_result_int(context, 1);

  GEOMBLOB_FUNC_END
}

static void ST_CoordDim(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  WKB_FUNC_START(spatialdb, context, args, geomblob, wkb, stream, error)
  sqlite3_result_int(context, geom_coord_dim(wkb.coord_type));
  WKB_FUNC_END
}

static void ST_GeometryType(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  WKB_FUNC_START(spatialdb, context, args, geomblob, wkb, stream, error)
  const char *type_name = geom_type_name(wkb.geom_type);
  if (type_name) {
    sqlite3_result_text(context, type_name, -1, SQLITE_STATIC);
  } else {
    sqlite3_result_error(context, "Unknown geometry type", -1);
  }
  WKB_FUNC_END
}

static void ST_AsBinary(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  GEOMBLOB_FUNC_START(spatialdb, context, args, geomblob, stream, error)
  sqlite3_result_blob(context, binstream_data(&stream), (int) binstream_available(&stream), SQLITE_TRANSIENT);
  GEOMBLOB_FUNC_END
}

static void ST_GeomFromWKB(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  const spatialdb_t *spatialdb = (const spatialdb_t *)sqlite3_user_data(context);
  BLOB_FUNC_START(context, args, stream, error)

  int32_t srid = -1;
  if (nbArgs == 2) {
    srid = (int32_t) sqlite3_value_int(args[1]);
  }

  geom_blob_writer_t writer;
  spatialdb->writer_init(&writer, srid);

  int result = spatialdb->read_geometry(&stream, geom_blob_writer_geom_consumer(&writer), &error);

  if (result != SQLITE_OK) {
    sqlite3_result_error(context, error_count(&error) > 0 ? error_message(&error) : "Could not parse WKB", -1);
  } else {
    sqlite3_result_blob(context, geom_blob_writer_getdata(&writer), (int) geom_blob_writer_length(&writer), SQLITE_TRANSIENT);
  }

  spatialdb->writer_destroy(&writer);

  BLOB_FUNC_END
}

static void ST_AsText(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  GEOMBLOB_FUNC_START(spatialdb, context, args, geomblob, stream, error)

  wkt_writer_t writer;
  wkt_writer_init(&writer);
  int result = spatialdb->read_geometry(&stream, wkt_writer_geom_consumer(&writer), &error);

  if (result != SQLITE_OK) {
    sqlite3_result_error(context, error_count(&error) > 0 ? error_message(&error) : "Could not parse WKB", -1);
  } else {
    sqlite3_result_text(context, wkt_writer_getwkt(&writer), (int) wkt_writer_length(&writer), SQLITE_TRANSIENT);
  }
  wkt_writer_destroy(&writer);

  GEOMBLOB_FUNC_END
}

static void ST_GeomFromText(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  const spatialdb_t *spatialdb = (const spatialdb_t *)sqlite3_user_data(context);
  TEXT_FUNC_START(context, args, text, error)

  int32_t srid = -1;
  if (nbArgs == 2) {
    srid = (int32_t) sqlite3_value_int(args[1]);
  }

  geom_blob_writer_t writer;
  spatialdb->writer_init(&writer, srid);
  int result = wkt_read_geometry(text, length, geom_blob_writer_geom_consumer(&writer), &error);

  if (result != SQLITE_OK) {
    sqlite3_result_error(context, error_count(&error) > 0 ? error_message(&error) : "Could not parse WKT", -1);
  } else {
    sqlite3_result_blob(context, geom_blob_writer_getdata(&writer), (int) geom_blob_writer_length(&writer), SQLITE_TRANSIENT);
  }

  spatialdb->writer_destroy(&writer);

  TEXT_FUNC_END
}

static void ST_WKBFromText(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  TEXT_FUNC_START(context, args, text, error)

  wkb_writer_t writer;
  wkb_writer_init(&writer, WKB_ISO);
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

static void SpatialDBType(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  const spatialdb_t *spatialdb = (const spatialdb_t *)sqlite3_user_data(context);
  sqlite3_result_text(context, spatialdb->name, -1, SQLITE_STATIC);
}

static void CheckSpatialDB(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  FUNCTION_TEXT_ARG(db_name)
  FUNCTION_START(context)

  if (nbArgs == 0) {
    FUNCTION_SET_TEXT_ARG(db_name, "main");
  } else {
    FUNCTION_GET_TEXT_ARG(context, db_name);
  }

  const spatialdb_t *spatialdb = (const spatialdb_t *)sqlite3_user_data(context);
  FUNCTION_RESULT = spatialdb->check(FUNCTION_DB_HANDLE, db_name, FUNCTION_ERROR_PTR);

  FUNCTION_END(context)
  FUNCTION_FREE_TEXT_ARG(db_name)
}

static void InitSpatialDB(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  FUNCTION_TEXT_ARG(db_name)
  FUNCTION_START(context)

  if (nbArgs == 0) {
    FUNCTION_SET_TEXT_ARG(db_name, "main");
  } else {
    FUNCTION_GET_TEXT_ARG(context, db_name);
  }

  FUNCTION_START_TRANSACTION(__initspatialdb);
  const spatialdb_t *spatialdb = (const spatialdb_t *)sqlite3_user_data(context);
  FUNCTION_RESULT = spatialdb->init(FUNCTION_DB_HANDLE, db_name, FUNCTION_ERROR_PTR);
  FUNCTION_END_TRANSACTION(__initspatialdb);

  FUNCTION_END(context)
  FUNCTION_FREE_TEXT_ARG(db_name)
}

/*
 * Supports the following parameter lists:
 * 4: table, column, type, srid
 * 5: db, table, column, type, srid
 * 6: table, column, type, srid, z, m
 * 7: db, table, column, type, srid, z, m
 */
static void AddGeometryColumn(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  FUNCTION_TEXT_ARG(db_name)
  FUNCTION_TEXT_ARG(table_name)
  FUNCTION_TEXT_ARG(column_name)
  FUNCTION_TEXT_ARG(geometry_type)
  FUNCTION_INT_ARG(srs_id)
  FUNCTION_INT_ARG(z)
  FUNCTION_INT_ARG(m)
  FUNCTION_START(context)

  if (nbArgs == 5 || nbArgs == 7) {
    FUNCTION_GET_TEXT_ARG(context, db_name)
  } else {
    FUNCTION_SET_TEXT_ARG(db_name, "main")
  }
  FUNCTION_GET_TEXT_ARG(context, table_name)
  FUNCTION_GET_TEXT_ARG(context, column_name)
  FUNCTION_GET_TEXT_ARG(context, geometry_type)
  FUNCTION_GET_INT_ARG(srs_id)
  if (nbArgs == 6 || nbArgs == 7) {
    FUNCTION_GET_INT_ARG(z)
    FUNCTION_GET_INT_ARG(m)
  } else {
    FUNCTION_SET_INT_ARG(z, 2)
    FUNCTION_SET_INT_ARG(m, 2)
  }

  FUNCTION_START_TRANSACTION(__add_geom_col)

  const spatialdb_t *spatialdb = (const spatialdb_t *)sqlite3_user_data(context);
  FUNCTION_RESULT = spatialdb->init(FUNCTION_DB_HANDLE, db_name, FUNCTION_ERROR_PTR);

  if (FUNCTION_RESULT == SQLITE_OK) {
    FUNCTION_RESULT = spatialdb->add_geometry_column(FUNCTION_DB_HANDLE, db_name, table_name, column_name, geometry_type, srs_id, z, m, FUNCTION_ERROR_PTR);
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

  const spatialdb_t *spatialdb = (const spatialdb_t *)sqlite3_user_data(context);
  if (spatialdb->create_tiles_table == NULL) {
    error_append(FUNCTION_ERROR_PTR, "Tiles tables are not supported in %s mode", spatialdb->name);
    goto exit;
  }

  FUNCTION_START_TRANSACTION(__create_tiles_table)
  FUNCTION_RESULT = spatialdb->init(FUNCTION_DB_HANDLE, db_name, FUNCTION_ERROR_PTR);
  if (FUNCTION_RESULT == SQLITE_OK) {
    FUNCTION_RESULT = spatialdb->create_tiles_table(FUNCTION_DB_HANDLE, db_name, table_name, FUNCTION_ERROR_PTR);
  }
  FUNCTION_END_TRANSACTION(__create_tiles_table)

  FUNCTION_END(context)
  FUNCTION_FREE_TEXT_ARG(db_name)
  FUNCTION_FREE_TEXT_ARG(table_name)
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

  const spatialdb_t *spatialdb = (const spatialdb_t *)sqlite3_user_data(context);
  if (spatialdb->create_spatial_index == NULL) {
    error_append(FUNCTION_ERROR_PTR, "Spatial indexes are not supported in %s mode", spatialdb->name);
    goto exit;
  }

  FUNCTION_START_TRANSACTION(__create_spatial_index)
  FUNCTION_RESULT = spatialdb->init(FUNCTION_DB_HANDLE, db_name, FUNCTION_ERROR_PTR);
  if (FUNCTION_RESULT == SQLITE_OK) {
    FUNCTION_RESULT = spatialdb->create_spatial_index(FUNCTION_DB_HANDLE, db_name, table_name, column_name, FUNCTION_ERROR_PTR);
  }
  FUNCTION_END_TRANSACTION(__create_spatial_index)

  FUNCTION_END(context)
  FUNCTION_FREE_TEXT_ARG(db_name)
  FUNCTION_FREE_TEXT_ARG(table_name)
}

#define REGISTER_FUNC(name, function, args, config) \
  if (sqlite3_create_function_v2( db, #name, args, SQLITE_UTF8, (void *) config, function, NULL, NULL, NULL ) != SQLITE_OK) { \
    printf("Error registering function %s/%d: %s\n", #name, args, sqlite3_errmsg(db));\
    return SQLITE_ERROR; \
  }
#define FUNC(name, args, config) REGISTER_FUNC(name, name, args, config)
#define ST_FUNC(name, args, config) REGISTER_FUNC(name, ST_##name, args, config) REGISTER_FUNC(ST_##name, ST_##name, args, config)
#define ST_ALIAS(name, function, args, config) REGISTER_FUNC(name, ST_##function, args, config) REGISTER_FUNC(ST_##name, ST_##function, args, config)

int spatialdb_init(sqlite3 *db, const char **pzErrMsg, const sqlite3_api_routines *pThunk, gpkg_schema schema) {
  SQLITE_EXTENSION_INIT2(pThunk)

  if (sqlite3_libversion_number() < 3007000) {
    if (pzErrMsg) {
      *pzErrMsg = sqlite3_mprintf("libgpkg requires SQLite 3.7.0 or higher; detected %s", sqlite3_libversion());
    }
    return SQLITE_ERROR;
  }

  const spatialdb_t *spatialdb;
  if (schema == SPATIALITE4) {
    spatialdb = &SPATIALITE4_DB;
  } else {
    spatialdb = &GEOPACKAGE_DB;
  }

  ST_FUNC(MinX, 1, spatialdb);
  ST_FUNC(MaxX, 1, spatialdb);
  ST_FUNC(MinY, 1, spatialdb);
  ST_FUNC(MaxY, 1, spatialdb);
  ST_FUNC(MinZ, 1, spatialdb);
  ST_FUNC(MaxZ, 1, spatialdb);
  ST_FUNC(MinM, 1, spatialdb);
  ST_FUNC(MaxM, 1, spatialdb);
  ST_FUNC(SRID, 1, spatialdb);
  ST_FUNC(SRID, 2, spatialdb);
  ST_FUNC(Is3d, 1, spatialdb);
  ST_FUNC(IsEmpty, 1, spatialdb);
  ST_FUNC(IsMeasured, 1, spatialdb);
  ST_FUNC(IsValid, 1, spatialdb);
  ST_FUNC(CoordDim, 1, spatialdb);
  ST_FUNC(GeometryType, 1, spatialdb);
  ST_FUNC(AsBinary, 1, spatialdb);
  ST_FUNC(GeomFromWKB, 1, spatialdb);
  ST_ALIAS(WKBToSQL, GeomFromWKB, 1, spatialdb);
  ST_FUNC(GeomFromWKB, 2, spatialdb);
  ST_FUNC(AsText, 1, spatialdb);
  ST_FUNC(GeomFromText, 1, spatialdb);
  ST_FUNC(WKBFromText, 1, spatialdb);
  ST_ALIAS(WKTToSQL, GeomFromText, 1, spatialdb);
  ST_FUNC(GeomFromText, 2, spatialdb);
  FUNC(CheckSpatialDB, 0, spatialdb);
  FUNC(CheckSpatialDB, 1, spatialdb);
  FUNC(InitSpatialDB, 0, spatialdb);
  FUNC(InitSpatialDB, 1, spatialdb);
  FUNC(AddGeometryColumn, 4, spatialdb);
  FUNC(AddGeometryColumn, 5, spatialdb);
  FUNC(AddGeometryColumn, 6, spatialdb);
  FUNC(AddGeometryColumn, 7, spatialdb);
  FUNC(CreateTilesTable, 1, spatialdb);
  FUNC(CreateTilesTable, 2, spatialdb);
  FUNC(CreateSpatialIndex, 2, spatialdb);
  FUNC(CreateSpatialIndex, 3, spatialdb);
  FUNC(SpatialDBType, 0, spatialdb);

  return SQLITE_OK;
}