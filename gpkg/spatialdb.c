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
#include "geom_func.h"
#include "sql.h"
#include "sqlite.h"
#include "spatialdb_internal.h"
#include "wkb.h"
#include "wkt.h"

SQLITE_EXTENSION_INIT1

#define ST_MIN_MAX(name, check, field) static void ST_##name(sqlite3_context *context, int nbArgs, sqlite3_value **args) { \
    FUNCTION_SPATIALDB_ARG(spatialdb); \
    FUNCTION_GEOM_ARG(geomblob); \
\
    FUNCTION_START_STATIC(context, 256); \
    FUNCTION_GET_SPATIALDB_ARG(context, spatialdb); \
    FUNCTION_GET_GEOM_ARG_UNSAFE(context, spatialdb, geomblob, 0); \
 \
    if (geomblob.envelope.check == 0) { \
        if (spatialdb->fill_envelope(&FUNCTION_GEOM_ARG_STREAM(geomblob), &geomblob.envelope, &FUNCTION_ERROR) != SQLITE_OK) { \
            if ( error_count(&FUNCTION_ERROR) == 0 ) error_append(&FUNCTION_ERROR, "Invalid geometry blob header");\
            goto exit; \
        } \
    } \
\
    if (geomblob.envelope.check) { \
        sqlite3_result_double(context, geomblob.envelope.field); \
    } else { \
        sqlite3_result_null(context); \
    } \
    FUNCTION_END(context); \
    FUNCTION_FREE_SPATIALDB_ARG(spatialdb); \
    FUNCTION_FREE_GEOM_ARG(geomblob); \
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
  FUNCTION_SPATIALDB_ARG(spatialdb);
  FUNCTION_GEOM_ARG(geomblob);

  FUNCTION_START_STATIC(context, 256);
  FUNCTION_GET_SPATIALDB_ARG(context, spatialdb);
  FUNCTION_GET_GEOM_ARG_UNSAFE(context, spatialdb, geomblob, 0);

  if (nbArgs == 1) {
    sqlite3_result_int(context, geomblob.srid);
  } else {
    FUNCTION_GET_INT_ARG(geomblob.srid, 1);
    if (binstream_seek(&FUNCTION_GEOM_ARG_STREAM(geomblob), 0) != SQLITE_OK) {
      sqlite3_result_error(context, "Error writing geometry blob header", -1);
      goto exit;
    }
    if (spatialdb->write_blob_header(&FUNCTION_GEOM_ARG_STREAM(geomblob), &geomblob, &FUNCTION_ERROR) != SQLITE_OK) {
      if (error_count(&FUNCTION_ERROR) == 0) {
        error_append(&FUNCTION_ERROR, "Error writing geometry blob header");
      }
      goto exit;
    }
    binstream_seek(&FUNCTION_GEOM_ARG_STREAM(geomblob), 0);
    sqlite3_result_blob(context, binstream_data(&FUNCTION_GEOM_ARG_STREAM(geomblob)), (int) binstream_available(&FUNCTION_GEOM_ARG_STREAM(geomblob)), SQLITE_TRANSIENT);
  }

  FUNCTION_END(context);
  FUNCTION_FREE_SPATIALDB_ARG(spatialdb);
  FUNCTION_FREE_GEOM_ARG(geomblob);
}

static void ST_IsEmpty(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  FUNCTION_SPATIALDB_ARG(spatialdb);
  FUNCTION_GEOM_ARG(geomblob);

  FUNCTION_START_STATIC(context, 256);
  FUNCTION_GET_SPATIALDB_ARG(context, spatialdb);
  FUNCTION_GET_GEOM_ARG_UNSAFE(context, spatialdb, geomblob, 0);

  sqlite3_result_int(context, geomblob.empty);

  FUNCTION_END(context);
  FUNCTION_FREE_SPATIALDB_ARG(spatialdb);
  FUNCTION_FREE_GEOM_ARG(geomblob);
}

static void ST_IsMeasured(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  FUNCTION_SPATIALDB_ARG(spatialdb);
  FUNCTION_WKB_ARG(wkb);

  FUNCTION_START_STATIC(context, 256);
  FUNCTION_GET_SPATIALDB_ARG(context, spatialdb);
  FUNCTION_GET_WKB_ARG_UNSAFE(context, spatialdb, wkb, 0);

  sqlite3_result_int(context, wkb.coord_type == GEOM_XYM || wkb.coord_type == GEOM_XYZM);

  FUNCTION_END(context);
  FUNCTION_FREE_SPATIALDB_ARG(spatialdb);
  FUNCTION_FREE_WKB_ARG(wkb);
}

static void ST_Is3d(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  FUNCTION_SPATIALDB_ARG(spatialdb);
  FUNCTION_WKB_ARG(wkb);

  FUNCTION_START_STATIC(context, 256);
  FUNCTION_GET_SPATIALDB_ARG(context, spatialdb);
  FUNCTION_GET_WKB_ARG_UNSAFE(context, spatialdb, wkb, 0);

  sqlite3_result_int(context, wkb.coord_type == GEOM_XYZ || wkb.coord_type == GEOM_XYZM);

  FUNCTION_END(context);
  FUNCTION_FREE_SPATIALDB_ARG(spatialdb);
  FUNCTION_FREE_WKB_ARG(wkb);
}

static void ST_IsValid(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  FUNCTION_SPATIALDB_ARG(spatialdb);
  FUNCTION_GEOM_ARG(geomblob);

  FUNCTION_START_STATIC(context, 256);
  FUNCTION_GET_SPATIALDB_ARG(context, spatialdb);
  FUNCTION_GET_GEOM_ARG_UNSAFE(context, spatialdb, geomblob, 0);

  geom_consumer_t consumer;
  geom_consumer_init(&consumer, NULL, NULL, NULL, NULL, NULL);
  if (spatialdb->read_geometry(&FUNCTION_GEOM_ARG_STREAM(geomblob), &consumer, NULL) != SQLITE_OK) {
    sqlite3_result_int(context, 0);
    goto exit;
  }

  sqlite3_result_int(context, 1);

  FUNCTION_END(context);
  FUNCTION_FREE_SPATIALDB_ARG(spatialdb);
  FUNCTION_FREE_GEOM_ARG(geomblob);
}

static void ST_CoordDim(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  FUNCTION_SPATIALDB_ARG(spatialdb);
  FUNCTION_WKB_ARG(wkb);

  FUNCTION_START_STATIC(context, 256);
  FUNCTION_GET_SPATIALDB_ARG(context, spatialdb);
  FUNCTION_GET_WKB_ARG_UNSAFE(context, spatialdb, wkb, 0);

  sqlite3_result_int(context, geom_coord_dim(wkb.coord_type));

  FUNCTION_END(context);
  FUNCTION_FREE_SPATIALDB_ARG(spatialdb);
  FUNCTION_FREE_WKB_ARG(wkb);
}

static void ST_GeometryType(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  FUNCTION_SPATIALDB_ARG(spatialdb);
  FUNCTION_WKB_ARG(wkb);

  FUNCTION_START_STATIC(context, 256);
  FUNCTION_GET_SPATIALDB_ARG(context, spatialdb);
  FUNCTION_GET_WKB_ARG_UNSAFE(context, spatialdb, wkb, 0);

  const char *type_name;
  if (geom_type_name(wkb.geom_type, &type_name) == SQLITE_OK) {
    sqlite3_result_text(context, type_name, -1, SQLITE_STATIC);
  } else {
    error_append(&FUNCTION_ERROR, "Unknown geometry type: %d", wkb.geom_type);
  }

  FUNCTION_END(context);
  FUNCTION_FREE_SPATIALDB_ARG(spatialdb);
  FUNCTION_FREE_WKB_ARG(wkb);
}

static void ST_AsBinary(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  FUNCTION_SPATIALDB_ARG(spatialdb);
  FUNCTION_GEOM_ARG(geomblob);

  FUNCTION_START_STATIC(context, 256);
  FUNCTION_GET_SPATIALDB_ARG(context, spatialdb);
  FUNCTION_GET_GEOM_ARG_UNSAFE(context, spatialdb, geomblob, 0);

  sqlite3_result_blob(context, binstream_data(&FUNCTION_GEOM_ARG_STREAM(geomblob)), (int) binstream_available(&FUNCTION_GEOM_ARG_STREAM(geomblob)), SQLITE_TRANSIENT);

  FUNCTION_END(context);
  FUNCTION_FREE_SPATIALDB_ARG(spatialdb);
  FUNCTION_FREE_GEOM_ARG(geomblob);
}

static void ST_GeomFromWKB(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  FUNCTION_SPATIALDB_ARG(spatialdb);
  FUNCTION_STREAM_ARG(wkb);
  FUNCTION_INT_ARG(srid);

  FUNCTION_START_STATIC(context, 256);
  FUNCTION_GET_SPATIALDB_ARG(context, spatialdb);
  FUNCTION_GET_STREAM_ARG_UNSAFE(context, wkb, 0);

  geom_blob_writer_t writer;
  if (nbArgs == 2) {
    FUNCTION_GET_INT_ARG(srid, 1);
    spatialdb->writer_init_srid(&writer, srid);
  } else {
    FUNCTION_SET_INT_ARG(srid, -1);
    spatialdb->writer_init(&writer);
  }

  FUNCTION_RESULT = wkb_read_geometry(&wkb, WKB_ISO, geom_blob_writer_geom_consumer(&writer), &FUNCTION_ERROR);
  if (FUNCTION_RESULT == SQLITE_OK) {
    sqlite3_result_blob(context, geom_blob_writer_getdata(&writer), (int) geom_blob_writer_length(&writer), sqlite3_free);
    spatialdb->writer_destroy(&writer, 0);
  } else {
    spatialdb->writer_destroy(&writer, 1);
  }

  FUNCTION_END(context);
  FUNCTION_FREE_SPATIALDB_ARG(spatialdb);
  FUNCTION_FREE_STREAM_ARG(wkb);
  FUNCTION_FREE_INT_ARG(srid);
}

static void ST_AsText(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  FUNCTION_SPATIALDB_ARG(spatialdb);
  FUNCTION_GEOM_ARG(geomblob);

  FUNCTION_START_STATIC(context, 256);
  FUNCTION_GET_SPATIALDB_ARG(context, spatialdb);
  FUNCTION_GET_GEOM_ARG_UNSAFE(context, spatialdb, geomblob, 0);

  wkt_writer_t writer;
  wkt_writer_init(&writer);

  FUNCTION_RESULT = spatialdb->read_geometry(&FUNCTION_GEOM_ARG_STREAM(geomblob), wkt_writer_geom_consumer(&writer), &FUNCTION_ERROR);

  if (FUNCTION_RESULT != SQLITE_OK) {
    if (error_count(&FUNCTION_ERROR) == 0) {
      error_append(&FUNCTION_ERROR, "Could not parse WKB");
    }
  } else {
    sqlite3_result_text(context, wkt_writer_getwkt(&writer), (int) wkt_writer_length(&writer), SQLITE_TRANSIENT);
  }
  wkt_writer_destroy(&writer);

  FUNCTION_END(context);
  FUNCTION_FREE_SPATIALDB_ARG(spatialdb);
  FUNCTION_FREE_GEOM_ARG(geomblob);
}

static void ST_GeomFromText(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  FUNCTION_SPATIALDB_ARG(spatialdb);
  FUNCTION_TEXT_ARG(wkt);
  FUNCTION_INT_ARG(srid);

  FUNCTION_START_STATIC(context, 256);
  FUNCTION_GET_SPATIALDB_ARG(context, spatialdb);
  FUNCTION_GET_TEXT_ARG_UNSAFE(wkt, 0);

  geom_blob_writer_t writer;
  if (nbArgs == 2) {
    FUNCTION_GET_INT_ARG(srid, 1);
    spatialdb->writer_init_srid(&writer, srid);
  } else {
    FUNCTION_SET_INT_ARG(srid, -1);
    spatialdb->writer_init(&writer);
  }

  FUNCTION_RESULT = wkt_read_geometry(wkt, FUNCTION_TEXT_ARG_LENGTH(wkt), geom_blob_writer_geom_consumer(&writer), &FUNCTION_ERROR);
  if (FUNCTION_RESULT == SQLITE_OK) {
    sqlite3_result_blob(context, geom_blob_writer_getdata(&writer), (int) geom_blob_writer_length(&writer), sqlite3_free);
    spatialdb->writer_destroy(&writer, 0);
  } else {
    spatialdb->writer_destroy(&writer, 1);
  }

  FUNCTION_END(context);
  FUNCTION_FREE_SPATIALDB_ARG(spatialdb);
  FUNCTION_FREE_TEXT_ARG(wkt);
  FUNCTION_FREE_INT_ARG(srid);
}

static void ST_WKBFromText(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  FUNCTION_SPATIALDB_ARG(spatialdb);
  FUNCTION_TEXT_ARG(wkt);

  FUNCTION_START_STATIC(context, 256);
  FUNCTION_GET_SPATIALDB_ARG(context, spatialdb);
  FUNCTION_GET_TEXT_ARG_UNSAFE(wkt, 0);

  wkb_writer_t writer;
  wkb_writer_init(&writer, WKB_ISO);

  FUNCTION_RESULT = wkt_read_geometry(wkt, FUNCTION_TEXT_ARG_LENGTH(wkt), wkb_writer_geom_consumer(&writer), &FUNCTION_ERROR);
  if (FUNCTION_RESULT == SQLITE_OK) {
    sqlite3_result_blob(context, wkb_writer_getwkb(&writer), (int) wkb_writer_length(&writer), sqlite3_free);
    wkb_writer_destroy(&writer, 0);
  } else {
    wkb_writer_destroy(&writer, 1);
  }

  FUNCTION_END(context);
  FUNCTION_FREE_SPATIALDB_ARG(spatialdb);
  FUNCTION_FREE_TEXT_ARG(wkt);
}

static void GPKG_IsAssignable(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  FUNCTION_TEXT_ARG(expected_type_name);
  FUNCTION_TEXT_ARG(actual_type_name);

  FUNCTION_START(context);
  FUNCTION_GET_TEXT_ARG(context, expected_type_name, 0);
  FUNCTION_GET_TEXT_ARG(context, actual_type_name, 1);

  geom_type_t expected_type;
  FUNCTION_RESULT = geom_type_from_string(expected_type_name, &expected_type);
  if (FUNCTION_RESULT != SQLITE_OK) {
    error_append(&FUNCTION_ERROR, "Invalid geometry type %s", expected_type_name);
    goto exit;
  }

  geom_type_t actual_type;
  FUNCTION_RESULT = geom_type_from_string(actual_type_name, &actual_type);
  if (FUNCTION_RESULT != SQLITE_OK) {
    error_append(&FUNCTION_ERROR, "Invalid geometry type %s", actual_type_name);
    goto exit;
  }

  sqlite3_result_int(context, geom_is_assignable(expected_type, actual_type));

  FUNCTION_END(context);
  FUNCTION_FREE_TEXT_ARG(expected_type_name);
  FUNCTION_FREE_TEXT_ARG(actual_type_name);
}

static void GPKG_SpatialDBType(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  FUNCTION_SPATIALDB_ARG(spatialdb);

  FUNCTION_START(context);
  FUNCTION_GET_SPATIALDB_ARG(context, spatialdb);

  sqlite3_result_text(context, spatialdb->name, -1, SQLITE_STATIC);

  FUNCTION_END(context);
  FUNCTION_FREE_SPATIALDB_ARG(spatialdb);
}

static void GPKG_CheckSpatialMetaData(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  FUNCTION_SPATIALDB_ARG(spatialdb);
  FUNCTION_TEXT_ARG(db_name);
  FUNCTION_INT_ARG(check);
  FUNCTION_INT_ARG(type);
  FUNCTION_START(context);

  check = 0;

  FUNCTION_GET_SPATIALDB_ARG(context, spatialdb);
  if (nbArgs == 0) {
    FUNCTION_SET_TEXT_ARG(db_name, "main");
  } else if (nbArgs == 1) {
    FUNCTION_GET_TYPE(type, 0);
    if (type == SQLITE_TEXT) {
      FUNCTION_GET_TEXT_ARG(context, db_name, 0);
    } else {
      FUNCTION_SET_TEXT_ARG(db_name, "main");
      FUNCTION_GET_INT_ARG(check, 0);
    }
  }  else {
    FUNCTION_GET_TEXT_ARG(context, db_name, 0);
    FUNCTION_GET_INT_ARG(check, 1);
  }

  FUNCTION_RESULT = spatialdb->check_meta(FUNCTION_DB_HANDLE, db_name, check, &FUNCTION_ERROR);
  if (FUNCTION_RESULT == SQLITE_OK) {
    sqlite3_result_null(context);
  }

  FUNCTION_END(context);
  FUNCTION_FREE_SPATIALDB_ARG(spatialdb);
  FUNCTION_FREE_TEXT_ARG(db_name);
  FUNCTION_FREE_INT_ARG(check);
  FUNCTION_FREE_INT_ARG(type);
}

static void GPKG_InitSpatialMetaData(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  FUNCTION_SPATIALDB_ARG(spatialdb);
  FUNCTION_TEXT_ARG(db_name);

  FUNCTION_START(context);
  FUNCTION_GET_SPATIALDB_ARG(context, spatialdb);
  if (nbArgs == 0) {
    FUNCTION_SET_TEXT_ARG(db_name, "main");
  } else {
    FUNCTION_GET_TEXT_ARG(context, db_name, 0);
  }

  FUNCTION_START_TRANSACTION(__initspatialdb);
  FUNCTION_RESULT = spatialdb->init_meta(FUNCTION_DB_HANDLE, db_name, &FUNCTION_ERROR);
  FUNCTION_END_TRANSACTION(__initspatialdb);

  if (FUNCTION_RESULT == SQLITE_OK) {
    sqlite3_result_null(context);
  }

  FUNCTION_END(context);
  FUNCTION_FREE_SPATIALDB_ARG(spatialdb);
  FUNCTION_FREE_TEXT_ARG(db_name);
}

/*
 * Supports the following parameter lists:
 * 4: table, column, type, srid
 * 5: db, table, column, type, srid
 * 6: table, column, type, srid, z, m
 * 7: db, table, column, type, srid, z, m
 */
static void GPKG_AddGeometryColumn(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  FUNCTION_SPATIALDB_ARG(spatialdb);
  FUNCTION_TEXT_ARG(db_name);
  FUNCTION_TEXT_ARG(table_name);
  FUNCTION_TEXT_ARG(column_name);
  FUNCTION_TEXT_ARG(geometry_type);
  FUNCTION_INT_ARG(srs_id);
  FUNCTION_INT_ARG(z);
  FUNCTION_INT_ARG(m);
  FUNCTION_START(context);

  FUNCTION_GET_SPATIALDB_ARG(context, spatialdb);
  if (nbArgs == 4) {
    FUNCTION_SET_TEXT_ARG(db_name, "main");
    FUNCTION_GET_TEXT_ARG(context, table_name, 0);
    FUNCTION_GET_TEXT_ARG(context, column_name, 1);
    FUNCTION_GET_TEXT_ARG(context, geometry_type, 2);
    FUNCTION_GET_INT_ARG(srs_id, 3);
    FUNCTION_SET_INT_ARG(z, 2);
    FUNCTION_SET_INT_ARG(m, 2);
  } else if (nbArgs == 5) {
    FUNCTION_GET_TEXT_ARG(context, db_name, 0);
    FUNCTION_GET_TEXT_ARG(context, table_name, 1);
    FUNCTION_GET_TEXT_ARG(context, column_name, 2);
    FUNCTION_GET_TEXT_ARG(context, geometry_type, 3);
    FUNCTION_GET_INT_ARG(srs_id, 4);
  } else if (nbArgs == 6) {
    FUNCTION_SET_TEXT_ARG(db_name, "main");
    FUNCTION_GET_TEXT_ARG(context, table_name, 0);
    FUNCTION_GET_TEXT_ARG(context, column_name, 1);
    FUNCTION_GET_TEXT_ARG(context, geometry_type, 2);
    FUNCTION_GET_INT_ARG(srs_id, 3);
    FUNCTION_GET_INT_ARG(z, 4);
    FUNCTION_GET_INT_ARG(m, 5);
  } else {
    FUNCTION_GET_TEXT_ARG(context, db_name, 0);
    FUNCTION_GET_TEXT_ARG(context, table_name, 1);
    FUNCTION_GET_TEXT_ARG(context, column_name, 2);
    FUNCTION_GET_TEXT_ARG(context, geometry_type, 3);
    FUNCTION_GET_INT_ARG(srs_id, 4);
    FUNCTION_GET_INT_ARG(z, 5);
    FUNCTION_GET_INT_ARG(m, 6);
  }

  FUNCTION_START_TRANSACTION(__add_geom_col);

  FUNCTION_RESULT = spatialdb->init_meta(FUNCTION_DB_HANDLE, db_name, &FUNCTION_ERROR);

  if (FUNCTION_RESULT == SQLITE_OK) {
    FUNCTION_RESULT = spatialdb->add_geometry_column(FUNCTION_DB_HANDLE, db_name, table_name, column_name, geometry_type, srs_id, z, m, &FUNCTION_ERROR);
  }

  FUNCTION_END_TRANSACTION(__add_geom_col);

  if (FUNCTION_RESULT == SQLITE_OK) {
    sqlite3_result_null(context);
  }

  FUNCTION_END(context);
  FUNCTION_FREE_SPATIALDB_ARG(spatialdb);
  FUNCTION_FREE_TEXT_ARG(db_name);
  FUNCTION_FREE_TEXT_ARG(table_name);
  FUNCTION_FREE_TEXT_ARG(column_name);
  FUNCTION_FREE_TEXT_ARG(geometry_type);
  FUNCTION_FREE_INT_ARG(srid);
  FUNCTION_FREE_INT_ARG(z);
  FUNCTION_FREE_INT_ARG(m);
}

static void GPKG_CreateTilesTable(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  FUNCTION_SPATIALDB_ARG(spatialdb);
  FUNCTION_TEXT_ARG(db_name);
  FUNCTION_TEXT_ARG(table_name);
  FUNCTION_START(context);

  FUNCTION_GET_SPATIALDB_ARG(context, spatialdb);
  if (nbArgs == 2) {
    FUNCTION_GET_TEXT_ARG(context, db_name, 0);
    FUNCTION_GET_TEXT_ARG(context, table_name, 1);
  } else {
    FUNCTION_SET_TEXT_ARG(db_name, "main");
    FUNCTION_GET_TEXT_ARG(context, table_name, 0);
  }

  if (spatialdb->create_tiles_table == NULL) {
    error_append(&FUNCTION_ERROR, "Tiles tables are not supported in %s mode", spatialdb->name);
    goto exit;
  }

  FUNCTION_START_TRANSACTION(__create_tiles_table);

  FUNCTION_RESULT = spatialdb->init_meta(FUNCTION_DB_HANDLE, db_name, &FUNCTION_ERROR);
  if (FUNCTION_RESULT == SQLITE_OK) {
    FUNCTION_RESULT = spatialdb->create_tiles_table(FUNCTION_DB_HANDLE, db_name, table_name, &FUNCTION_ERROR);
  }

  FUNCTION_END_TRANSACTION(__create_tiles_table);

  if (FUNCTION_RESULT == SQLITE_OK) {
    sqlite3_result_null(context);
  }

  FUNCTION_END(context);
  FUNCTION_FREE_SPATIALDB_ARG(spatialdb);
  FUNCTION_FREE_TEXT_ARG(db_name);
  FUNCTION_FREE_TEXT_ARG(table_name);
}

static void GPKG_CreateSpatialIndex(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  FUNCTION_SPATIALDB_ARG(spatialdb);
  FUNCTION_TEXT_ARG(db_name);
  FUNCTION_TEXT_ARG(table_name);
  FUNCTION_TEXT_ARG(column_name);
  FUNCTION_START(context);

  FUNCTION_GET_SPATIALDB_ARG(context, spatialdb);
  if (nbArgs == 3) {
    FUNCTION_GET_TEXT_ARG(context, db_name, 0);
    FUNCTION_GET_TEXT_ARG(context, table_name, 1);
    FUNCTION_GET_TEXT_ARG(context, column_name, 2);
  } else {
    FUNCTION_SET_TEXT_ARG(db_name, "main");
    FUNCTION_GET_TEXT_ARG(context, table_name, 0);
    FUNCTION_GET_TEXT_ARG(context, column_name, 1);
  }

  if (spatialdb->create_spatial_index == NULL) {
    error_append(&FUNCTION_ERROR, "Spatial indexes are not supported in %s mode", spatialdb->name);
    goto exit;
  }

  FUNCTION_START_TRANSACTION(__create_spatial_index);

  FUNCTION_RESULT = spatialdb->init_meta(FUNCTION_DB_HANDLE, db_name, &FUNCTION_ERROR);
  if (FUNCTION_RESULT == SQLITE_OK) {
    FUNCTION_RESULT = spatialdb->create_spatial_index(FUNCTION_DB_HANDLE, db_name, table_name, column_name, &FUNCTION_ERROR);
  }

  FUNCTION_END_TRANSACTION(__create_spatial_index);

  if (FUNCTION_RESULT == SQLITE_OK) {
    sqlite3_result_null(context);
  }

  FUNCTION_END(context);
  FUNCTION_FREE_SPATIALDB_ARG(spatialdb);
  FUNCTION_FREE_TEXT_ARG(db_name);
  FUNCTION_FREE_TEXT_ARG(table_name);
  FUNCTION_FREE_TEXT_ARG(column_name);
}

const spatialdb_t *spatialdb_detect_schema(sqlite3 *db) {
  char error_message[256];
  error_t error;
  error_init_fixed(&error, error_message, 256);

  const spatialdb_t *schemas[] = {
    spatialdb_geopackage_schema(),
    spatialdb_spatialite4_schema(),
    spatialdb_spatialite3_schema(),
    NULL
  };

  const spatialdb_t **schema = &schemas[0];
  while (*schema != NULL) {
    error_reset(&error);
    (*schema)->check_meta(db, "main", 0, &error);
    if (error_count(&error) == 0) {
      break;
    } else {
      schema++;
    }
  }

  if (*schema != NULL) {
    return *schema;
  } else {
    return schemas[0];
  }
}

int spatialdb_init(sqlite3 *db, const char **pzErrMsg, const sqlite3_api_routines *pThunk, const spatialdb_t *spatialdb) {
  SQLITE_EXTENSION_INIT2(pThunk)

  if (sqlite3_libversion_number() < 3007000) {
    if (pzErrMsg) {
      *pzErrMsg = sqlite3_mprintf("libgpkg requires SQLite 3.7.0 or higher; detected %s", sqlite3_libversion());
    }
    return SQLITE_ERROR;
  }

  error_t error;
  if (error_init(&error) != SQLITE_OK) {
    if (pzErrMsg) {
      *pzErrMsg = sqlite3_mprintf("Could not initialize error buffer");
    }
    return SQLITE_ERROR;
  }

  if (spatialdb == NULL) {
    spatialdb = spatialdb_detect_schema(db);
  }

  if (spatialdb->init != NULL) {
    spatialdb->init(db, spatialdb, &error);
  }

  REG_FUNC(ST, MinX, 1, spatialdb, &error);
  REG_FUNC(ST, MaxX, 1, spatialdb, &error);
  REG_FUNC(ST, MinY, 1, spatialdb, &error);
  REG_FUNC(ST, MaxY, 1, spatialdb, &error);
  REG_FUNC(ST, MinZ, 1, spatialdb, &error);
  REG_FUNC(ST, MaxZ, 1, spatialdb, &error);
  REG_FUNC(ST, MinM, 1, spatialdb, &error);
  REG_FUNC(ST, MaxM, 1, spatialdb, &error);
  REG_FUNC(ST, SRID, 1, spatialdb, &error);
  REG_FUNC(ST, SRID, 2, spatialdb, &error);
  REG_FUNC(ST, Is3d, 1, spatialdb, &error);
  REG_FUNC(ST, IsEmpty, 1, spatialdb, &error);
  REG_FUNC(ST, IsMeasured, 1, spatialdb, &error);
  REG_FUNC(ST, IsValid, 1, spatialdb, &error);
  REG_FUNC(ST, CoordDim, 1, spatialdb, &error);
  REG_FUNC(ST, GeometryType, 1, spatialdb, &error);
  REG_FUNC(ST, AsBinary, 1, spatialdb, &error);
  REG_FUNC(ST, GeomFromWKB, 1, spatialdb, &error);
  REG_ALIAS(ST, WKBToSQL, GeomFromWKB, 1, spatialdb, &error);
  REG_FUNC(ST, GeomFromWKB, 2, spatialdb, &error);
  REG_FUNC(ST, AsText, 1, spatialdb, &error);
  REG_FUNC(ST, GeomFromText, 1, spatialdb, &error);
  REG_FUNC(ST, WKBFromText, 1, spatialdb, &error);
  REG_ALIAS(ST, WKTToSQL, GeomFromText, 1, spatialdb, &error);
  REG_FUNC(ST, GeomFromText, 2, spatialdb, &error);

  REG_FUNC(GPKG, IsAssignable, 2, spatialdb, &error);
  REG_FUNC(GPKG, CheckSpatialMetaData, 0, spatialdb, &error);
  REG_FUNC(GPKG, CheckSpatialMetaData, 1, spatialdb, &error);
  REG_FUNC(GPKG, CheckSpatialMetaData, 2, spatialdb, &error);
  REG_FUNC(GPKG, InitSpatialMetaData, 0, spatialdb, &error);
  REG_FUNC(GPKG, InitSpatialMetaData, 1, spatialdb, &error);
  REG_FUNC(GPKG, AddGeometryColumn, 4, spatialdb, &error);
  REG_FUNC(GPKG, AddGeometryColumn, 5, spatialdb, &error);
  REG_FUNC(GPKG, AddGeometryColumn, 6, spatialdb, &error);
  REG_FUNC(GPKG, AddGeometryColumn, 7, spatialdb, &error);
  REG_FUNC(GPKG, CreateTilesTable, 1, spatialdb, &error);
  REG_FUNC(GPKG, CreateTilesTable, 2, spatialdb, &error);
  REG_FUNC(GPKG, CreateSpatialIndex, 2, spatialdb, &error);
  REG_FUNC(GPKG, CreateSpatialIndex, 3, spatialdb, &error);
  REG_FUNC(GPKG, SpatialDBType, 0, spatialdb, &error);

  geom_func_init(db, spatialdb, &error);

  int result;
  if (error_count(&error) == 0) {
    result = SQLITE_OK;
  } else {
    if (pzErrMsg) {
      *pzErrMsg = sqlite3_mprintf("%s", error_message(&error));
    }
    result = SQLITE_ERROR;
  }
  error_destroy(&error);
  return result;
}
