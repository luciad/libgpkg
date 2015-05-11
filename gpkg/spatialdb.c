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
#include <sqlite3.h>
#include <sys/types.h>
#include <errno.h>
#include "error.h"
#include "atomic_ops.h"
#include "binstream.h"
#include "blobio.h"
#ifdef GPKG_HAVE_CONFIG_H
#include "config.h"
#endif
#include "geomio.h"
#include "geom_func.h"
#include "i18n.h"
#include "sql.h"
#include "sqlite.h"
#include "spatialdb_internal.h"
#include "wkb.h"
#include "wkt.h"

#define ST_MIN_MAX(name, check, field) static void ST_##name(sqlite3_context *context, int nbArgs, sqlite3_value **args) { \
    spatialdb_t *spatialdb; \
    FUNCTION_GEOM_ARG(geomblob); \
\
    FUNCTION_START_STATIC(context, 256); \
    spatialdb = (spatialdb_t *)sqlite3_user_data(context); \
    FUNCTION_GET_GEOM_ARG_UNSAFE(context, spatialdb, geomblob, 0); \
 \
    if (geomblob.envelope.check == 0) { \
        if (spatialdb->fill_envelope(&FUNCTION_GEOM_ARG_STREAM(geomblob), &geomblob.envelope, FUNCTION_ERROR) != SQLITE_OK) { \
            if ( error_count(FUNCTION_ERROR) == 0 ) error_append(FUNCTION_ERROR, "Invalid geometry blob header");\
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
  spatialdb_t *spatialdb;
  FUNCTION_GEOM_ARG(geomblob);

  FUNCTION_START_STATIC(context, 256);
  spatialdb = (spatialdb_t *)sqlite3_user_data(context);
  FUNCTION_GET_GEOM_ARG_UNSAFE(context, spatialdb, geomblob, 0);

  if (nbArgs == 1) {
    sqlite3_result_int(context, geomblob.srid);
  } else {
    FUNCTION_GET_INT_ARG(geomblob.srid, 1);
    if (binstream_seek(&FUNCTION_GEOM_ARG_STREAM(geomblob), 0) != SQLITE_OK) {
      sqlite3_result_error(context, "Error writing geometry blob header", -1);
      goto exit;
    }
    if (spatialdb->write_blob_header(&FUNCTION_GEOM_ARG_STREAM(geomblob), &geomblob, FUNCTION_ERROR) != SQLITE_OK) {
      if (error_count(FUNCTION_ERROR) == 0) {
        error_append(FUNCTION_ERROR, "Error writing geometry blob header");
      }
      goto exit;
    }
    binstream_seek(&FUNCTION_GEOM_ARG_STREAM(geomblob), 0);
    sqlite3_result_blob(context, binstream_data(&FUNCTION_GEOM_ARG_STREAM(geomblob)), (int) binstream_available(&FUNCTION_GEOM_ARG_STREAM(geomblob)), SQLITE_TRANSIENT);
  }

  FUNCTION_END(context);
  FUNCTION_FREE_GEOM_ARG(geomblob);
}

static void ST_IsEmpty(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  spatialdb_t *spatialdb;
  FUNCTION_GEOM_ARG(geomblob);

  FUNCTION_START_STATIC(context, 256);
  spatialdb = (spatialdb_t *)sqlite3_user_data(context);
  FUNCTION_GET_GEOM_ARG_UNSAFE(context, spatialdb, geomblob, 0);

  sqlite3_result_int(context, geomblob.empty);

  FUNCTION_END(context);

  FUNCTION_FREE_GEOM_ARG(geomblob);
}

static void ST_IsMeasured(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  spatialdb_t *spatialdb;
  FUNCTION_WKB_ARG(wkb);

  FUNCTION_START_STATIC(context, 256);
  spatialdb = (spatialdb_t *)sqlite3_user_data(context);
  FUNCTION_GET_WKB_ARG_UNSAFE(context, spatialdb, wkb, 0);

  sqlite3_result_int(context, wkb.coord_type == GEOM_XYM || wkb.coord_type == GEOM_XYZM);

  FUNCTION_END(context);
  FUNCTION_FREE_WKB_ARG(wkb);
}

static void ST_Is3d(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  spatialdb_t *spatialdb;
  FUNCTION_WKB_ARG(wkb);

  FUNCTION_START_STATIC(context, 256);
  spatialdb = (spatialdb_t *)sqlite3_user_data(context);
  FUNCTION_GET_WKB_ARG_UNSAFE(context, spatialdb, wkb, 0);

  sqlite3_result_int(context, wkb.coord_type == GEOM_XYZ || wkb.coord_type == GEOM_XYZM);

  FUNCTION_END(context);
  FUNCTION_FREE_WKB_ARG(wkb);
}

static void ST_CoordDim(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  spatialdb_t *spatialdb;
  FUNCTION_WKB_ARG(wkb);

  FUNCTION_START_STATIC(context, 256);
  spatialdb = (spatialdb_t *)sqlite3_user_data(context);
  FUNCTION_GET_WKB_ARG_UNSAFE(context, spatialdb, wkb, 0);

  sqlite3_result_int(context, geom_coord_dim(wkb.coord_type));

  FUNCTION_END(context);

  FUNCTION_FREE_WKB_ARG(wkb);
}

static void ST_GeometryType(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  spatialdb_t *spatialdb;
  FUNCTION_WKB_ARG(wkb);

  FUNCTION_START_STATIC(context, 256);
  spatialdb = (spatialdb_t *)sqlite3_user_data(context);
  FUNCTION_GET_WKB_ARG_UNSAFE(context, spatialdb, wkb, 0);

  const char *type_name;
  if (geom_type_name(wkb.geom_type, &type_name) == SQLITE_OK) {
    sqlite3_result_text(context, type_name, -1, SQLITE_STATIC);
  } else {
    error_append(FUNCTION_ERROR, "Unknown geometry type: %d", wkb.geom_type);
  }

  FUNCTION_END(context);

  FUNCTION_FREE_WKB_ARG(wkb);
}

typedef struct {
  uint8_t *data;
  int length;
} geom_blob_auxdata;

static geom_blob_auxdata *geom_blob_auxdata_malloc() {
  return (geom_blob_auxdata *)sqlite3_malloc(sizeof(geom_blob_auxdata));
}

static void geom_blob_auxdata_free(void *auxdata) {
  if (auxdata != NULL) {
    geom_blob_auxdata *geom = (geom_blob_auxdata *)auxdata;
    sqlite3_free(geom->data);
    geom->data = NULL;
    sqlite3_free(geom);
  }
}

static void ST_AsBinary(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  spatialdb_t *spatialdb;
  FUNCTION_GEOM_ARG(geomblob);

  FUNCTION_START_STATIC(context, 256);
  spatialdb = (spatialdb_t *)sqlite3_user_data(context);
  FUNCTION_GET_GEOM_ARG_UNSAFE(context, spatialdb, geomblob, 0);

  wkb_writer_t writer;
  wkb_writer_init(&writer, WKB_ISO);

  FUNCTION_RESULT = spatialdb->read_geometry(&FUNCTION_GEOM_ARG_STREAM(geomblob), wkb_writer_geom_consumer(&writer), FUNCTION_ERROR);

  if (FUNCTION_RESULT == SQLITE_OK) {
    sqlite3_result_blob(context, wkb_writer_getwkb(&writer), (int) wkb_writer_length(&writer), sqlite3_free);
    wkb_writer_destroy(&writer, 0);
  } else {
    wkb_writer_destroy(&writer, 1);
  }

  FUNCTION_END(context);

  FUNCTION_FREE_GEOM_ARG(geomblob);
}

static void ST_AsText(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  spatialdb_t *spatialdb;
  FUNCTION_GEOM_ARG(geomblob);

  FUNCTION_START_STATIC(context, 256);
  spatialdb = (spatialdb_t *)sqlite3_user_data(context);
  FUNCTION_GET_GEOM_ARG_UNSAFE(context, spatialdb, geomblob, 0);

  wkt_writer_t writer;
  wkt_writer_init(&writer);

  FUNCTION_RESULT = spatialdb->read_geometry(&FUNCTION_GEOM_ARG_STREAM(geomblob), wkt_writer_geom_consumer(&writer), FUNCTION_ERROR);

  if (FUNCTION_RESULT == SQLITE_OK) {
    sqlite3_result_text(context, wkt_writer_getwkt(&writer), (int) wkt_writer_length(&writer), SQLITE_TRANSIENT);
  }
  wkt_writer_destroy(&writer);

  FUNCTION_END(context);

  FUNCTION_FREE_GEOM_ARG(geomblob);
}

static int geometry_is_assignable(geom_type_t expected, geom_type_t actual, errorstream_t* error) {
  if (!geom_is_assignable(expected, actual)) {
    const char* expectedName = NULL;
    const char* actualName = NULL;
    if (geom_type_name(expected, &expectedName) == SQLITE_OK && geom_type_name(actual, &actualName) == SQLITE_OK) {
      error_append(error, "Incorrect geometry type. Expected '%d' actual '%s'", expectedName, actualName);
    } else {
      error_append(error, "Incorrect geometry type");
    }
    return SQLITE_ERROR;
  } else {
    return SQLITE_OK;
  }
}

typedef int (*geometry_constructor_func)(sqlite3_context *context, void *user_data, geom_consumer_t *consumer, int nbArgs, sqlite3_value **args, errorstream_t *error);

static void geometry_constructor(sqlite3_context *context, const spatialdb_t *spatialdb, geometry_constructor_func constructor, void* user_data, geom_type_t requiredType, int nbArgs, sqlite3_value **args) {
  FUNCTION_START_STATIC(context, 256);

  geom_blob_auxdata *geom = (geom_blob_auxdata *)sqlite3_get_auxdata(context, 0);

  if (geom == NULL) {
    geom_blob_writer_t writer;

    if (sqlite3_value_type(args[nbArgs - 1]) == SQLITE_INTEGER) {
      spatialdb->writer_init_srid(&writer, sqlite3_value_int(args[nbArgs - 1]));
      nbArgs -= 1;
    } else {
      spatialdb->writer_init(&writer);
    }

    FUNCTION_RESULT = constructor(context, user_data, geom_blob_writer_geom_consumer(&writer), nbArgs, args, FUNCTION_ERROR);

    if (FUNCTION_RESULT == SQLITE_OK) {
      if (geometry_is_assignable(requiredType, writer.geom_type, FUNCTION_ERROR) == SQLITE_OK) {
        uint8_t *data = geom_blob_writer_getdata(&writer);
        int length = (int) geom_blob_writer_length(&writer);
        sqlite3_result_blob(context, data, length, SQLITE_TRANSIENT);
        spatialdb->writer_destroy(&writer, 0);

        geom = geom_blob_auxdata_malloc();
        if (geom != NULL) {
          geom->data = data;
          geom->length = length;
          sqlite3_set_auxdata(context, 0, geom, geom_blob_auxdata_free);
        }
      }
    } else {
      spatialdb->writer_destroy(&writer, 1);
    }
  } else {
    sqlite3_result_blob(context, geom->data, geom->length, SQLITE_TRANSIENT);
  }

  FUNCTION_END(context);
}

static int geom_from_wkb(sqlite3_context *context, void *user_data, geom_consumer_t* consumer, int nbArgs, sqlite3_value **args, errorstream_t *error) {
  FUNCTION_STREAM_ARG(wkb);
  FUNCTION_START_NESTED(context, error);
  FUNCTION_GET_STREAM_ARG_UNSAFE(context, wkb, 0);

  FUNCTION_RESULT = wkb_read_geometry(&wkb, WKB_ISO, consumer, FUNCTION_ERROR);

  FUNCTION_END_NESTED(context);
  FUNCTION_FREE_STREAM_ARG(wkb);

  return FUNCTION_RESULT;
}

static void ST_GeomFromWKB(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  spatialdb_t *spatialdb = (spatialdb_t *)sqlite3_user_data(context);
  geometry_constructor(context, spatialdb, geom_from_wkb, NULL, GEOM_GEOMETRY, nbArgs, args);
}

typedef struct {
  volatile long ref_count;
  const spatialdb_t *spatialdb;
  i18n_locale_t *locale;
} fromtext_t;

static fromtext_t *fromtext_init(const spatialdb_t *spatialdb) {
  fromtext_t *ctx = (fromtext_t *)sqlite3_malloc(sizeof(fromtext_t));

  if (ctx == NULL) {
    return NULL;
  }

  i18n_locale_t *locale = i18n_locale_init("C");
  if (locale == NULL) {
    sqlite3_free(ctx);
    return NULL;
  }

  ctx->ref_count = 1;
  ctx->locale = locale;
  ctx->spatialdb = spatialdb;
  return ctx;
}

static void fromtext_acquire(fromtext_t *fromtext) {
  if (fromtext) {
    atomic_inc_long(&fromtext->ref_count);
  }
}

static void fromtext_release(fromtext_t *fromtext) {
  if (fromtext) {
    long newval = atomic_dec_long(&fromtext->ref_count);
    if (newval == 0) {
      i18n_locale_destroy(fromtext->locale);
      fromtext->locale = NULL;
      sqlite3_free(fromtext);
    }
  }
}

static int geom_from_wkt(sqlite3_context *context, void *user_data, geom_consumer_t* consumer, int nbArgs, sqlite3_value **args, errorstream_t *error) {
  FUNCTION_TEXT_ARG(wkt);
  FUNCTION_START_NESTED(context, error);

  FUNCTION_GET_TEXT_ARG_UNSAFE(wkt, 0);

  FUNCTION_RESULT = wkt_read_geometry(wkt, FUNCTION_TEXT_ARG_LENGTH(wkt), consumer, (i18n_locale_t *)user_data, FUNCTION_ERROR);

  FUNCTION_END_NESTED(context);
  FUNCTION_FREE_TEXT_ARG(wkt);

  return FUNCTION_RESULT;
}

static void ST_GeomFromText(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  fromtext_t *fromtext = (fromtext_t *)sqlite3_user_data(context);
  geometry_constructor(context, fromtext->spatialdb, geom_from_wkt, fromtext->locale, GEOM_GEOMETRY, nbArgs, args);
}

static int point_from_coords(sqlite3_context *context, void *user_data, geom_consumer_t *consumer, int nbArgs, sqlite3_value **args, errorstream_t *error) {
  int result = SQLITE_OK;

  if (nbArgs < 2 || nbArgs > 4) {
    error_append(error, "Invalid number of coordinates: %d", nbArgs);
    result = SQLITE_ERROR;
  } else {
    double coord[4];
    for(int i = 0; i < nbArgs; i++) {
      coord[i] = sqlite3_value_double(args[i]);
    }

    geom_header_t header;
    header.geom_type = GEOM_POINT;
    if (nbArgs == 2) {
      header.coord_type = GEOM_XY;
      header.coord_size = 2;
    } else if (nbArgs == 3) {
      header.coord_type = GEOM_XYZ;
      header.coord_size = 3;
    } else {
      header.coord_type = GEOM_XYZM;
      header.coord_size = 4;
    }

    if (result == SQLITE_OK) {
      result = consumer->begin(consumer, error);
    }
    if (result == SQLITE_OK) {
      result = consumer->begin_geometry(consumer, &header, error);
    }
    if (result == SQLITE_OK) {
      result = consumer->coordinates(consumer, &header, 1, coord, 0, error);
    }
    if (result == SQLITE_OK) {
      result = consumer->end_geometry(consumer, &header, error);
    }
    if (result == SQLITE_OK) {
      result = consumer->end(consumer, error);
    }
  }

  return result;
}

static void ST_Point(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  fromtext_t *fromtext = (fromtext_t *)sqlite3_user_data(context);
  if (sqlite3_value_type(args[0]) == SQLITE_TEXT) {
    geometry_constructor(context, fromtext->spatialdb, geom_from_wkt, fromtext->locale, GEOM_POINT, nbArgs, args);
  } else if (sqlite3_value_type(args[0]) == SQLITE_BLOB) {
    geometry_constructor(context, fromtext->spatialdb, geom_from_wkb, NULL, GEOM_POINT, nbArgs, args);
  } else {
    geometry_constructor(context, fromtext->spatialdb, point_from_coords, NULL, GEOM_POINT, nbArgs, args);
  }
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
    error_append(FUNCTION_ERROR, "Invalid geometry type %s", expected_type_name);
    goto exit;
  }

  geom_type_t actual_type;
  FUNCTION_RESULT = geom_type_from_string(actual_type_name, &actual_type);
  if (FUNCTION_RESULT != SQLITE_OK) {
    error_append(FUNCTION_ERROR, "Invalid geometry type %s", actual_type_name);
    goto exit;
  }

  sqlite3_result_int(context, geom_is_assignable(expected_type, actual_type));

  FUNCTION_END(context);
  FUNCTION_FREE_TEXT_ARG(expected_type_name);
  FUNCTION_FREE_TEXT_ARG(actual_type_name);
}

static void GPKG_SpatialDBType(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  spatialdb_t *spatialdb;

  FUNCTION_START(context);
  spatialdb = (spatialdb_t *)sqlite3_user_data(context);

  sqlite3_result_text(context, spatialdb->name, -1, SQLITE_STATIC);

  FUNCTION_END(context);
}

static void GPKG_CheckSpatialMetaData(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  spatialdb_t *spatialdb;
  FUNCTION_TEXT_ARG(db_name);
  FUNCTION_INT_ARG(check);
  FUNCTION_INT_ARG(type);
  FUNCTION_START(context);

  spatialdb = (spatialdb_t *)sqlite3_user_data(context);
  if (nbArgs == 0) {
    FUNCTION_SET_TEXT_ARG(db_name, "main");
    FUNCTION_SET_INT_ARG(check, 0);
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

  if (check != 0) {
    check = SQL_CHECK_ALL;
  }

  FUNCTION_RESULT = spatialdb->check_meta(FUNCTION_DB_HANDLE, db_name, check, FUNCTION_ERROR);
  if (FUNCTION_RESULT == SQLITE_OK) {
    sqlite3_result_null(context);
  }

  FUNCTION_END(context);

  FUNCTION_FREE_TEXT_ARG(db_name);
  FUNCTION_FREE_INT_ARG(check);
  FUNCTION_FREE_INT_ARG(type);
}

static void GPKG_InitSpatialMetaData(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  spatialdb_t *spatialdb;
  FUNCTION_TEXT_ARG(db_name);

  FUNCTION_START(context);
  spatialdb = (spatialdb_t *)sqlite3_user_data(context);
  if (nbArgs == 0) {
    FUNCTION_SET_TEXT_ARG(db_name, "main");
  } else {
    FUNCTION_GET_TEXT_ARG(context, db_name, 0);
  }

  FUNCTION_START_TRANSACTION(__initspatialdb);
  FUNCTION_RESULT = spatialdb->init_meta(FUNCTION_DB_HANDLE, db_name, FUNCTION_ERROR);
  FUNCTION_END_TRANSACTION(__initspatialdb);

  if (FUNCTION_RESULT == SQLITE_OK) {
    sqlite3_result_null(context);
  }

  FUNCTION_END(context);

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
  spatialdb_t *spatialdb;
  FUNCTION_TEXT_ARG(db_name);
  FUNCTION_TEXT_ARG(table_name);
  FUNCTION_TEXT_ARG(column_name);
  FUNCTION_TEXT_ARG(geometry_type);
  FUNCTION_INT_ARG(srs_id);
  FUNCTION_INT_ARG(z);
  FUNCTION_INT_ARG(m);
  FUNCTION_START(context);

  spatialdb = (spatialdb_t *)sqlite3_user_data(context);
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

  FUNCTION_RESULT = spatialdb->init_meta(FUNCTION_DB_HANDLE, db_name, FUNCTION_ERROR);

  if (FUNCTION_RESULT == SQLITE_OK) {
    FUNCTION_RESULT = spatialdb->add_geometry_column(FUNCTION_DB_HANDLE, db_name, table_name, column_name, geometry_type, srs_id, z, m, FUNCTION_ERROR);
  }

  FUNCTION_END_TRANSACTION(__add_geom_col);

  if (FUNCTION_RESULT == SQLITE_OK) {
    sqlite3_result_null(context);
  }

  FUNCTION_END(context);

  FUNCTION_FREE_TEXT_ARG(db_name);
  FUNCTION_FREE_TEXT_ARG(table_name);
  FUNCTION_FREE_TEXT_ARG(column_name);
  FUNCTION_FREE_TEXT_ARG(geometry_type);
  FUNCTION_FREE_INT_ARG(srid);
  FUNCTION_FREE_INT_ARG(z);
  FUNCTION_FREE_INT_ARG(m);
}

static void GPKG_CreateTilesTable(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  spatialdb_t *spatialdb;
  FUNCTION_TEXT_ARG(db_name);
  FUNCTION_TEXT_ARG(table_name);
  FUNCTION_START(context);

  spatialdb = (spatialdb_t *)sqlite3_user_data(context);
  if (nbArgs == 2) {
    FUNCTION_GET_TEXT_ARG(context, db_name, 0);
    FUNCTION_GET_TEXT_ARG(context, table_name, 1);
  } else {
    FUNCTION_SET_TEXT_ARG(db_name, "main");
    FUNCTION_GET_TEXT_ARG(context, table_name, 0);
  }

  if (spatialdb->create_tiles_table == NULL) {
    error_append(FUNCTION_ERROR, "Tiles tables are not supported in %s mode", spatialdb->name);
    goto exit;
  }

  FUNCTION_START_TRANSACTION(__create_tiles_table);

  FUNCTION_RESULT = spatialdb->init_meta(FUNCTION_DB_HANDLE, db_name, FUNCTION_ERROR);
  if (FUNCTION_RESULT == SQLITE_OK) {
    FUNCTION_RESULT = spatialdb->create_tiles_table(FUNCTION_DB_HANDLE, db_name, table_name, FUNCTION_ERROR);
  }

  FUNCTION_END_TRANSACTION(__create_tiles_table);

  if (FUNCTION_RESULT == SQLITE_OK) {
    sqlite3_result_null(context);
  }

  FUNCTION_END(context);

  FUNCTION_FREE_TEXT_ARG(db_name);
  FUNCTION_FREE_TEXT_ARG(table_name);
}

static void GPKG_CreateSpatialIndex(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  spatialdb_t *spatialdb;
  FUNCTION_TEXT_ARG(db_name);
  FUNCTION_TEXT_ARG(table_name);
  FUNCTION_TEXT_ARG(geometry_column_name);
  FUNCTION_TEXT_ARG(id_column_name);
  FUNCTION_START(context);

  spatialdb = (spatialdb_t *)sqlite3_user_data(context);
  if (nbArgs == 4) {
    FUNCTION_GET_TEXT_ARG(context, db_name, 0);
    FUNCTION_GET_TEXT_ARG(context, table_name, 1);
    FUNCTION_GET_TEXT_ARG(context, geometry_column_name, 2);
    FUNCTION_GET_TEXT_ARG(context, id_column_name, 3);
  } else {
    FUNCTION_SET_TEXT_ARG(db_name, "main");
    FUNCTION_GET_TEXT_ARG(context, table_name, 0);
    FUNCTION_GET_TEXT_ARG(context, geometry_column_name, 1);
    FUNCTION_GET_TEXT_ARG(context, id_column_name, 2);
  }

  if (spatialdb->create_spatial_index == NULL) {
    error_append(FUNCTION_ERROR, "Spatial indexes are not supported in %s mode", spatialdb->name);
    goto exit;
  }

  FUNCTION_START_TRANSACTION(__create_spatial_index);

  FUNCTION_RESULT = spatialdb->init_meta(FUNCTION_DB_HANDLE, db_name, FUNCTION_ERROR);
  if (FUNCTION_RESULT == SQLITE_OK) {
    FUNCTION_RESULT = spatialdb->create_spatial_index(FUNCTION_DB_HANDLE, db_name, table_name, geometry_column_name, id_column_name, FUNCTION_ERROR);
  }

  FUNCTION_END_TRANSACTION(__create_spatial_index);

  if (FUNCTION_RESULT == SQLITE_OK) {
    sqlite3_result_null(context);
  }

  FUNCTION_END(context);

  FUNCTION_FREE_TEXT_ARG(db_name);
  FUNCTION_FREE_TEXT_ARG(table_name);
  FUNCTION_FREE_TEXT_ARG(geometry_column_name);
  FUNCTION_FREE_TEXT_ARG(id_column_name);
}

const spatialdb_t *spatialdb_detect_schema(sqlite3 *db) {
  char message_buffer[256];
  errorstream_t error;
  error_init_fixed(&error, message_buffer, 256);

  const spatialdb_t *schemas[] = {
    spatialdb_geopackage_schema(),
    spatialdb_spatialite4_schema(),
    spatialdb_spatialite3_schema(),
    spatialdb_spatialite2_schema(),
    NULL
  };

  const spatialdb_t **schema = &schemas[0];
  while (*schema != NULL) {
    error_reset(&error);
    (*schema)->check_meta(db, "main", SQL_CHECK_PRIMARY_KEY | SQL_CHECK_NULLABLE, &error);
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

#define STR(x) #x

#define SPATIALDB_FUNCTION(db, pre, name, args, flags, spatialdb, err)                                                 \
  do {                                                                                                                 \
    sql_create_function(db, STR(name), pre##_##name, args, flags, (void*)spatialdb, NULL, err);                        \
    sql_create_function(db, STR(pre##_##name), pre##_##name, args, flags, (void*)spatialdb, NULL, err);                \
  } while (0)

#define SPATIALDB_ALIAS(db, pre, name, func, args, flags, spatialdb, err)                                              \
  do {                                                                                                                 \
    sql_create_function(db, STR(name), pre##_##func, args, flags, (void*)spatialdb, NULL, err);                        \
    sql_create_function(db, STR(pre##_##name), pre##_##func, args, flags, (void*)spatialdb, NULL, err);                \
  } while (0)

#define FROMTEXT_FUNCTION(db, pre, name, args, flags, ft, err)                                                         \
  do {                                                                                                                 \
    fromtext_acquire(fromtext);                                                                                        \
    sql_create_function(db, STR(name), pre##_##name, args, flags, ft, (void(*)(void*))fromtext_release, err);          \
    fromtext_acquire(fromtext);                                                                                        \
    sql_create_function(db, STR(pre##_##name), pre##_##name, args, flags, ft, (void(*)(void*))fromtext_release, err);  \
  } while (0)

#define FROMTEXT_ALIAS(db, pre, name, func, args, flags, ft, err)                                                      \
  do {                                                                                                                 \
    fromtext_acquire(fromtext);                                                                                        \
    sql_create_function(db, STR(name), pre##_##func, args, flags, (void*)ft, (void(*)(void*))fromtext_release, err);   \
    fromtext_acquire(fromtext);                                                                                        \
    sql_create_function(db, STR(pre##_##name), pre##_##func, args, flags, (void*)ft, (void(*)(void*))fromtext_release, err);  \
  } while (0)

SQLITE_EXTENSION_INIT1

int spatialdb_init(sqlite3 *db, const char **pzErrMsg, const sqlite3_api_routines *pThunk, const spatialdb_t *spatialdb) {
  SQLITE_EXTENSION_INIT2(pThunk)

  if (sqlite3_libversion_number() < 3007000) {
    if (pzErrMsg) {
      *pzErrMsg = sqlite3_mprintf("libgpkg requires SQLite 3.7.0 or higher; detected %s", sqlite3_libversion());
    }
    return SQLITE_ERROR;
  }

#if !defined(SQLITE_CORE)
  if (sqlite3_compileoption_used != NULL) {
#else
  if (1) {
#endif
    const char *forbidden_options[] = {
      "SQLITE_OMIT_FOREIGN_KEY", "foreign key",
      "SQLITE_OMIT_TRIGGER", "trigger",
      "SQLITE_OMIT_VIRTUALTABLE", "virtual table",
      "SQLITE_RTREE_INT_ONLY", "floating point rtree",
      NULL, NULL
    };

    const char **forbidden_option = forbidden_options;
    while (*forbidden_option != NULL) {
      if (sqlite3_compileoption_used(*forbidden_option)) {
        if (pzErrMsg) {
          *pzErrMsg = sqlite3_mprintf("libgpkg requires %s support but %s compile option was used", *(forbidden_option + 1), *forbidden_option);
        }
        return SQLITE_ERROR;
      }
      forbidden_option += 2;
    }

    const char *required_options[] = {
      "SQLITE_ENABLE_RTREE", "rtree",
      NULL, NULL
    };

    const char **required_option = required_options;
    while (*required_option != NULL) {
      if (!sqlite3_compileoption_used(*required_option)) {
        if (pzErrMsg) {
          *pzErrMsg = sqlite3_mprintf("libgpkg requires %s support but %s compile option was not used", *(required_option + 1), *required_option);
        }
        return SQLITE_ERROR;
      }
      required_option += 2;
    }
  }

  errorstream_t error;
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

  SPATIALDB_FUNCTION(db, ST, MinX, 1, SQL_DETERMINISTIC, spatialdb, &error);
  SPATIALDB_FUNCTION(db, ST, MaxX, 1, SQL_DETERMINISTIC, spatialdb, &error);
  SPATIALDB_FUNCTION(db, ST, MinY, 1, SQL_DETERMINISTIC, spatialdb, &error);
  SPATIALDB_FUNCTION(db, ST, MaxY, 1, SQL_DETERMINISTIC, spatialdb, &error);
  SPATIALDB_FUNCTION(db, ST, MinZ, 1, SQL_DETERMINISTIC, spatialdb, &error);
  SPATIALDB_FUNCTION(db, ST, MaxZ, 1, SQL_DETERMINISTIC, spatialdb, &error);
  SPATIALDB_FUNCTION(db, ST, MinM, 1, SQL_DETERMINISTIC, spatialdb, &error);
  SPATIALDB_FUNCTION(db, ST, MaxM, 1, SQL_DETERMINISTIC, spatialdb, &error);
  SPATIALDB_FUNCTION(db, ST, SRID, 1, SQL_DETERMINISTIC, spatialdb, &error);
  SPATIALDB_FUNCTION(db, ST, SRID, 2, SQL_DETERMINISTIC, spatialdb, &error);
  SPATIALDB_FUNCTION(db, ST, Is3d, 1, SQL_DETERMINISTIC, spatialdb, &error);
  SPATIALDB_FUNCTION(db, ST, IsEmpty, 1, SQL_DETERMINISTIC, spatialdb, &error);
  SPATIALDB_FUNCTION(db, ST, IsMeasured, 1, SQL_DETERMINISTIC, spatialdb, &error);
  SPATIALDB_FUNCTION(db, ST, CoordDim, 1, SQL_DETERMINISTIC, spatialdb, &error);
  SPATIALDB_FUNCTION(db, ST, GeometryType, 1, SQL_DETERMINISTIC, spatialdb, &error);
  SPATIALDB_FUNCTION(db, ST, AsBinary, 1, SQL_DETERMINISTIC, spatialdb, &error);
  SPATIALDB_FUNCTION(db, ST, GeomFromWKB, 1, SQL_DETERMINISTIC, spatialdb, &error);
  SPATIALDB_FUNCTION(db, ST, GeomFromWKB, 2, SQL_DETERMINISTIC, spatialdb, &error);
  SPATIALDB_ALIAS(db, ST, WKBToSQL, GeomFromWKB, 1, SQL_DETERMINISTIC, spatialdb, &error);
  SPATIALDB_ALIAS(db, ST, WKBToSQL, GeomFromWKB, 2, SQL_DETERMINISTIC, spatialdb, &error);
  SPATIALDB_FUNCTION(db, ST, AsText, 1, SQL_DETERMINISTIC, spatialdb, &error);

  fromtext_t *fromtext = fromtext_init(spatialdb);
  if (fromtext != NULL) {
    FROMTEXT_FUNCTION(db, ST, GeomFromText, 1, SQL_DETERMINISTIC, fromtext, &error);
    FROMTEXT_FUNCTION(db, ST, GeomFromText, 2, SQL_DETERMINISTIC, fromtext, &error);
    FROMTEXT_ALIAS(db, ST, WKTToSQL, GeomFromText, 1, SQL_DETERMINISTIC, fromtext, &error);
    FROMTEXT_ALIAS(db, ST, WKTToSQL, GeomFromText, 2, SQL_DETERMINISTIC, fromtext, &error);

    FROMTEXT_FUNCTION(db, ST, Point, 1, SQL_DETERMINISTIC, fromtext, &error);
    FROMTEXT_ALIAS(db, ST, MakePoint, Point, 1, SQL_DETERMINISTIC, fromtext, &error);
    FROMTEXT_FUNCTION(db, ST, Point, 2, SQL_DETERMINISTIC, fromtext, &error);
    FROMTEXT_ALIAS(db, ST, MakePoint, Point, 2, SQL_DETERMINISTIC, fromtext, &error);
    FROMTEXT_FUNCTION(db, ST, Point, 3, SQL_DETERMINISTIC, fromtext, &error);
    FROMTEXT_ALIAS(db, ST, MakePoint, Point, 3, SQL_DETERMINISTIC, fromtext, &error);
    FROMTEXT_FUNCTION(db, ST, Point, 4, SQL_DETERMINISTIC, fromtext, &error);
    FROMTEXT_ALIAS(db, ST, MakePoint, Point, 4, SQL_DETERMINISTIC, fromtext, &error);
    FROMTEXT_FUNCTION(db, ST, Point, 5, SQL_DETERMINISTIC, fromtext, &error);
    FROMTEXT_ALIAS(db, ST, MakePoint, Point, 5, SQL_DETERMINISTIC, fromtext, &error);

    fromtext_release(fromtext);
  } else {
    error_append(&error, "Could not create fromtext function context");
  }

  SPATIALDB_FUNCTION(db, GPKG, IsAssignable, 2, SQL_DETERMINISTIC, spatialdb, &error);
  SPATIALDB_FUNCTION(db, GPKG, CheckSpatialMetaData, 0, 0, spatialdb, &error);
  SPATIALDB_FUNCTION(db, GPKG, CheckSpatialMetaData, 1, 0, spatialdb, &error);
  SPATIALDB_FUNCTION(db, GPKG, CheckSpatialMetaData, 2, 0, spatialdb, &error);
  SPATIALDB_FUNCTION(db, GPKG, InitSpatialMetaData, 0, 0, spatialdb, &error);
  SPATIALDB_FUNCTION(db, GPKG, InitSpatialMetaData, 1, 0, spatialdb, &error);
  SPATIALDB_FUNCTION(db, GPKG, AddGeometryColumn, 4, 0, spatialdb, &error);
  SPATIALDB_FUNCTION(db, GPKG, AddGeometryColumn, 5, 0, spatialdb, &error);
  SPATIALDB_FUNCTION(db, GPKG, AddGeometryColumn, 6, 0, spatialdb, &error);
  SPATIALDB_FUNCTION(db, GPKG, AddGeometryColumn, 7, 0, spatialdb, &error);
  SPATIALDB_FUNCTION(db, GPKG, CreateTilesTable, 1, 0, spatialdb, &error);
  SPATIALDB_FUNCTION(db, GPKG, CreateTilesTable, 2, 0, spatialdb, &error);
  SPATIALDB_FUNCTION(db, GPKG, CreateSpatialIndex, 3, 0, spatialdb, &error);
  SPATIALDB_FUNCTION(db, GPKG, CreateSpatialIndex, 4, 0, spatialdb, &error);
  SPATIALDB_FUNCTION(db, GPKG, SpatialDBType, 0, 0, spatialdb, &error);


#ifdef GPKG_GEOM_FUNC
  geom_func_init(db, spatialdb, &error);
#endif

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
