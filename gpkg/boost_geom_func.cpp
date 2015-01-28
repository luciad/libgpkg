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
#include "boost_geom_io.hpp"
#include "boost_geom_wrapper.hpp"

extern "C" {
#include "error.h"
#include "geom_func.h"
#include "sql.h"
}

typedef struct {
  gpkg::GeometryPtr geometry;
  int srid;
} boostgeom_geometry_t;

static boostgeom_geometry_t *get_boost_geom(sqlite3_context *context, const spatialdb_t *spatialdb, sqlite3_value *value, errorstream_t *error) {
  geom_blob_header_t header;

  uint8_t *blob = (uint8_t *) sqlite3_value_blob(value);
  size_t blob_length = (size_t) sqlite3_value_bytes(value);

  if (blob == NULL) {
    return NULL;
  }

  binstream_t stream;
  binstream_init(&stream, blob, blob_length);

  spatialdb->read_blob_header(&stream, &header, error);

  boostgeom_writer_t writer;
  boostgeom_writer_init_srid(&writer, header.srid);

  spatialdb->read_geometry(&stream, boostgeom_writer_geom_consumer(&writer), error);

  gpkg::GeometryPtr g = boostgeom_writer_getgeometry(&writer);
  boostgeom_writer_destroy(&writer, g.which() == 0);

  if (g.which() == 0) {
    boostgeom_writer_destroy(&writer, true);
    return nullptr;
  } else {
    boostgeom_writer_destroy(&writer, false);
    boostgeom_geometry_t *geom = new boostgeom_geometry_t();
    geom->geometry = g;
    geom->srid = header.srid;
    return geom;
  }
}

static void free_boost_geom(void *data) {
  if (data == NULL) {
    return;
  }

  boostgeom_geometry_t *geom = (boostgeom_geometry_t *) data;
  if (geom != NULL) {
    gpkg::delete_geometry(geom->geometry);
    delete geom;
  }
}

static int set_boost_geom_result(sqlite3_context *context, const spatialdb_t *spatialdb, boostgeom_geometry_t *geom, errorstream_t *error) {
  int result = SQLITE_OK;

  if (geom == NULL) {
    sqlite3_result_null(context);
    return result;
  } else {
    geom_blob_writer_t writer;
    spatialdb->writer_init_srid(&writer, geom->srid);

    result = boostgeom_read_geometry(geom->geometry, geom_blob_writer_geom_consumer(&writer), error);

    if (result == SQLITE_OK) {
      sqlite3_result_blob(context, geom_blob_writer_getdata(&writer), geom_blob_writer_length(&writer), sqlite3_free);
    } else {
      sqlite3_result_error(context, error_message(error), -1);
    }

    spatialdb->writer_destroy(&writer, 0);

    return result;
  }
}

#define BOOSTGEOM_START(context) \
  const spatialdb_t *spatialdb = (const spatialdb_t *)sqlite3_user_data(context); \
  char error_buffer[256];\
  errorstream_t error;\
  error_init_fixed(&error, error_buffer, 256)

#define BOOSTGEOM_GET_GEOM(name, args, i) \
  boostgeom_geometry_t *name = (boostgeom_geometry_t *)sqlite3_get_auxdata(context, i); \
  int name##_set_auxdata = 0; \
  if (name == NULL) { \
    name = get_boost_geom( context, spatialdb, args[i], &error ); \
    name##_set_auxdata = 1;\
  }
#define BOOSTGEOM_FREE_GEOM(name, i) \
  if (name != NULL && name##_set_auxdata) { \
    sqlite3_set_auxdata(context, i, (void*)name, free_boost_geom); \
  }

#define BOOSTGEOM_GEOM__INT(sql_name, boost_name)                                                                      \
  static void ST_##sql_name(sqlite3_context *context, int nbArgs, sqlite3_value **args) {                              \
    BOOSTGEOM_START(context);                                                                                          \
    BOOSTGEOM_GET_GEOM(g1, args, 0);                                                                                   \
                                                                                                                       \
    if (g1 == NULL) {                                                                                                  \
      if (error_count(&error) > 0) {                                                                                   \
        sqlite3_result_error(context, error_message(&error), -1);                                                      \
      } else {                                                                                                         \
        sqlite3_result_null(context);                                                                                  \
      }                                                                                                                \
      return;                                                                                                          \
    }                                                                                                                  \
                                                                                                                       \
    int result = gpkg::boost_name(g1->geometry);                                                                       \
    sqlite3_result_int(context, result);                                                                               \
                                                                                                                       \
    BOOSTGEOM_FREE_GEOM(g1, 0);                                                                                        \
  }

#define BOOSTGEOM_GEOM__DOUBLE(sql_name, boost_name)                                                                   \
  static void ST_##sql_name(sqlite3_context *context, int nbArgs, sqlite3_value **args) {                              \
    BOOSTGEOM_START(context);                                                                                          \
    BOOSTGEOM_GET_GEOM(g1, args, 0);                                                                                   \
                                                                                                                       \
    if (g1 == NULL) {                                                                                                  \
      if (error_count(&error) > 0) {                                                                                   \
        sqlite3_result_error(context, error_message(&error), -1);                                                      \
      } else {                                                                                                         \
        sqlite3_result_null(context);                                                                                  \
      }                                                                                                                \
      return;                                                                                                          \
    }                                                                                                                  \
                                                                                                                       \
    double result = gpkg::boost_name(g1->geometry);                                                                    \
    sqlite3_result_double(context, result);                                                                            \
                                                                                                                       \
    BOOSTGEOM_FREE_GEOM(g1, 0);                                                                                        \
  }

BOOSTGEOM_GEOM__INT(IsValid, is_valid)
BOOSTGEOM_GEOM__INT(IsSimple, is_simple)

BOOSTGEOM_GEOM__DOUBLE(Area, area)

static void GPKG_BoostGeometryVersion(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
  int boost_major = BOOST_VERSION / 100000;
  int boost_minor = (BOOST_VERSION / 100) % 1000;
  int boost_subminor = BOOST_VERSION % 100;
  char *version = sqlite3_mprintf("%d.%d.%d", boost_major, boost_minor, boost_subminor);
  if (version) {
    sqlite3_result_text(context, version, -1, SQLITE_TRANSIENT);
    sqlite3_free(version);
  } else {
    sqlite3_result_error(context, "Could not obtain Boost.Geometry version number", -1);
  }
}

#define STR(x) #x

#define BOOSTGEOM_FUNCTION(db, prefix, name, nbArgs, ctx, error)                                                      \
  do {                                                                                                                 \
    sql_create_function(db, STR(name), prefix##_##name, nbArgs, SQL_DETERMINISTIC, (void*)ctx, NULL, error);           \
    sql_create_function(db, STR(prefix##_##name), prefix##_##name, nbArgs, SQL_DETERMINISTIC, (void*)ctx, NULL, error);\
  } while (0)

extern "C" {
void geom_func_init(sqlite3 *db, const spatialdb_t *spatialdb, errorstream_t *error) {
  BOOSTGEOM_FUNCTION(db, GPKG, BoostGeometryVersion, 0, spatialdb, error);
  BOOSTGEOM_FUNCTION(db, ST, IsValid, 1, spatialdb, error);
  BOOSTGEOM_FUNCTION(db, ST, IsSimple, 1, spatialdb, error);

  BOOSTGEOM_FUNCTION(db, ST, Area, 1, spatialdb, error);
}
}