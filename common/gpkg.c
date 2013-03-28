#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include "config.h"
#include "sqlite.h"
#include "binstream.h"
#include "geomio.h"
#include "gpb.h"
#include "sql.h"
#include "wkb.h"
#include "wkt.h"

#include "tables.h"

SQLITE_EXTENSION_INIT1

#define ST_MIN_MAX(name, check, field) static void ST_##name(sqlite3_context *context, int nbArgs, sqlite3_value **args) { \
    binstream_t stream; \
    gpb_t gpb; \
    \
    binstream_init( &stream, (uint8_t*)sqlite3_value_blob(args[0]), sqlite3_value_bytes(args[0]) ); \
    if (gpb_read_header( &stream, &gpb ) != SQLITE_OK) { \
        sqlite3_result_error(context, "Invalid GPB header", -1); \
        return; \
    } \
 \
    if (gpb.check == 0) { \
        if (gpb_envelope_init_from_wkb(&stream, &gpb) != SQLITE_OK) { \
            sqlite3_result_error(context, "Invalid GPB header", -1); \
            return; \
        } \
    } \
\
    if (gpb.check) { \
        sqlite3_result_double( context, gpb.field ); \
    } else { \
        sqlite3_result_error(context, "Geometry envelope does not contain " #name, -1); \
    } \
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
    binstream_t stream;
    gpb_t gpb;

    binstream_init(&stream, (uint8_t *) sqlite3_value_blob(args[0]), (size_t) sqlite3_value_bytes(args[0]));
    if (gpb_read_header(&stream, &gpb) != SQLITE_OK) {
        sqlite3_result_error(context, "Invalid GPB header", -1);
        return;
    }

    if (nbArgs == 1) {
        sqlite3_result_int(context, gpb.srid);
    } else {
        gpb.srid = sqlite3_value_int(args[1]);
        binstream_seek(&stream, 0);
        gpb_write_header(&stream, &gpb);
        binstream_seek(&stream, 0);
        sqlite3_result_blob(context, binstream_data(&stream), (int) binstream_available(&stream), SQLITE_TRANSIENT);
    }
}

static void ST_IsMeasured(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
    binstream_t stream;
    gpb_t gpb;

    binstream_init(&stream, (uint8_t *) sqlite3_value_blob(args[0]), (size_t) sqlite3_value_bytes(args[0]));
    if (gpb_read_header(&stream, &gpb) != SQLITE_OK) {
        sqlite3_result_error(context, "Invalid GPB header", -1);
        return;
    }

    geom_header_t wkb;
    wkb_read_header(&stream, &wkb);

    sqlite3_result_int(context, wkb.coord_type == GEOM_XYM || wkb.coord_type == GEOM_XYZM);
}

static void ST_Is3d(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
    binstream_t stream;
    gpb_t gpb;

    binstream_init(&stream, (uint8_t *) sqlite3_value_blob(args[0]), (size_t) sqlite3_value_bytes(args[0]));
    if (gpb_read_header(&stream, &gpb) != SQLITE_OK) {
        sqlite3_result_error(context, "Invalid GPB header", -1);
        return;
    }

    geom_header_t wkb;
    wkb_read_header(&stream, &wkb);

    sqlite3_result_int(context, wkb.coord_type == GEOM_XYZ || wkb.coord_type == GEOM_XYZM);
}

static void ST_IsValid(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
    binstream_t stream;
    gpb_t gpb;

    binstream_init(&stream, (uint8_t *) sqlite3_value_blob(args[0]), (size_t) sqlite3_value_bytes(args[0]));
    if (gpb_read_header(&stream, &gpb) != SQLITE_OK) {
        sqlite3_result_int(context, 0);
        return;
    }

    geom_reader_t reader;
    geom_reader_init(&reader, NULL, NULL, NULL);
    if (wkb_read_geometry(&stream, &reader) != SQLITE_OK) {
        sqlite3_result_int(context, 0);
        return;
    }

    sqlite3_result_int(context, 1);
}

static void ST_CoordDim(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
    binstream_t stream;
    gpb_t gpb;

    binstream_init(&stream, (uint8_t *) sqlite3_value_blob(args[0]), (size_t) sqlite3_value_bytes(args[0]));
    if (gpb_read_header(&stream, &gpb) != SQLITE_OK) {
        sqlite3_result_error(context, "Invalid GPB header", -1);
        return;
    }

    geom_header_t wkb;
    wkb_read_header(&stream, &wkb);
    sqlite3_result_int(context, geom_coord_dim(&wkb));
}

static void ST_GeometryType(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
    binstream_t stream;
    gpb_t gpb;

    binstream_init(&stream, (uint8_t *) sqlite3_value_blob(args[0]), (size_t) sqlite3_value_bytes(args[0]));
    if (gpb_read_header(&stream, &gpb) != SQLITE_OK) {
        sqlite3_result_error(context, "Invalid GPB header", -1);
        return;
    }

    geom_header_t wkb;
    wkb_read_header(&stream, &wkb);
    char *type_name = geom_type_name(&wkb);
    if (type_name) {
        sqlite3_result_text(context, type_name, -1, SQLITE_STATIC);
    } else {
        sqlite3_result_error(context, "Unknown geometry type", -1);
    }
}

static void ST_AsBinary(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
    binstream_t stream;
    gpb_t gpb;

    binstream_init(&stream, (uint8_t *) sqlite3_value_blob(args[0]), (size_t) sqlite3_value_bytes(args[0]));
    if (gpb_read_header(&stream, &gpb) != SQLITE_OK) {
        sqlite3_result_error(context, "Invalid GPB header", -1);
        return;
    }

    sqlite3_result_blob(context, binstream_data(&stream), (int) binstream_available(&stream), SQLITE_TRANSIENT);
}

static void ST_GeomFromWKB(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
    binstream_t stream;
    binstream_init(&stream, (uint8_t *) sqlite3_value_blob(args[0]), (size_t) sqlite3_value_bytes(args[0]));

    uint32_t srid = 0;
    if (nbArgs == 2) {
        srid = (uint32_t) sqlite3_value_int(args[1]);
    }

    gpb_writer_t writer;
    gpb_writer_init(&writer, srid);

    int result = wkb_read_geometry(&stream, &writer.geom_reader);

    if (result != SQLITE_OK) {
        sqlite3_result_error(context, "Could not parse WKB", -1);
    } else {
        sqlite3_result_blob(context, gpb_writer_getgpb(&writer), (int) gpb_writer_length(&writer), SQLITE_TRANSIENT);
    }
    gpb_writer_destroy(&writer);
}

static void ST_AsText(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
    binstream_t stream;
    gpb_t gpb;

    binstream_init(&stream, (uint8_t *) sqlite3_value_blob(args[0]), (size_t) sqlite3_value_bytes(args[0]));
    if (gpb_read_header(&stream, &gpb) != 0) {
        sqlite3_result_error(context, "Invalid GPB header", -1);
        return;
    }

    wkt_writer_t writer;
    wkt_writer_init(&writer);
    int result = wkb_read_geometry(&stream, &writer.geom_reader);
    if (result != SQLITE_OK) {
        sqlite3_result_error(context, "Could not parse WKB", -1);
    } else {
        sqlite3_result_text(context, wkt_writer_getwkt(&writer), (int) wkt_writer_length(&writer), SQLITE_TRANSIENT);
    }
    wkt_writer_destroy(&writer);
}

static void ST_GeomFromText(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
    char *text = (char *) sqlite3_value_text(args[0]);
    size_t length = (size_t) sqlite3_value_bytes(args[0]);

    uint32_t srid = 0;
    if (nbArgs == 2) {
        srid = (uint32_t) sqlite3_value_int(args[1]);
    }

    gpb_writer_t writer;
    gpb_writer_init(&writer, srid);
    int result = wkt_read_geometry(text, length, &writer.geom_reader);
    if (result != SQLITE_OK) {
        sqlite3_result_error(context, "Could not parse WKT", -1);
    } else {
        sqlite3_result_blob(context, gpb_writer_getgpb(&writer), (int) gpb_writer_length(&writer), SQLITE_TRANSIENT);
    }
    gpb_writer_destroy(&writer);
}

static void CheckGpkg(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
    int result;

    int errors = 0;
    strbuf_t errmsg;
    result = strbuf_init(&errmsg, 128);
    if (result != SQLITE_OK) {
        goto exit;
    }

    sqlite3 *db = sqlite3_context_db_handle(context);

    table_info_t **table = tables;
    while(*table != NULL) {
        result = sql_check_table(db, *table, &errors, &errmsg);
        if (result != SQLITE_OK) {
            goto exit;
        }
        table++;
    }

    exit:
    if (result == SQLITE_OK) {
        if (errors > 0) {
            sqlite3_result_error(context, strbuf_data_pointer(&errmsg), -1);
        } else {
            sqlite3_result_null(context);
        }
    } else {
        sqlite3_result_error_code(context, result);
    }
    strbuf_destroy(&errmsg);
}

static void InitGpkg(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
    int result;

    int errors = 0;
    strbuf_t errmsg;
    result = strbuf_init(&errmsg, 128);
    if (result != SQLITE_OK) {
        goto exit;
    }

    sqlite3 *db = sqlite3_context_db_handle(context);

    table_info_t **table = tables;
    while(*table != NULL) {
        result = sql_init_table(db, *table, &errors, &errmsg);
        if (result != SQLITE_OK) {
            goto exit;
        }
        table++;
    }

    exit:
    if (result == SQLITE_OK) {
        if (errors > 0) {
            sqlite3_result_error(context, strbuf_data_pointer(&errmsg), -1);
        } else {
            sqlite3_result_null(context);
        }
    } else {
        sqlite3_result_error_code(context, result);
    }
    strbuf_destroy(&errmsg);
}

const char *gpkg_libversion(void) {
    return VERSION;
}

#define REGISTER_FUNC(name, function, args) if (sqlite3_create_function_v2( db, #name, args, SQLITE_ANY, NULL, function, NULL, NULL, NULL ) != SQLITE_OK) return SQLITE_ERROR;
#define FUNC(name, args) REGISTER_FUNC(name, name, args)
#define ST_FUNC(name, args) REGISTER_FUNC(name, ST_##name, args) REGISTER_FUNC(ST_##name, ST_##name, args)
#define ST_ALIAS(name, function, args) REGISTER_FUNC(name, ST_##function, args) REGISTER_FUNC(ST_##name, ST_##function, args)

int gpkg_extension_init(sqlite3 *db, const char **pzErrMsg, const struct sqlite3_api_routines *pThunk) {
    SQLITE_EXTENSION_INIT2(pThunk)

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
    ST_FUNC( IsValid, 1 );
    ST_FUNC( IsMeasured, 1 );
    ST_FUNC( Is3d, 1 );
    ST_FUNC( CoordDim, 1 );
    ST_FUNC( GeometryType, 1 );
    ST_FUNC( AsBinary, 1 );
    ST_FUNC( GeomFromWKB, 1 );
    ST_ALIAS( WKBToSQL, GeomFromWKB, 1 );
    ST_FUNC( GeomFromWKB, 2 );
    ST_FUNC( AsText, 1 );
    ST_FUNC( GeomFromText, 1 );
    ST_ALIAS( WKTToSQL, GeomFromText, 1 );
    ST_FUNC( GeomFromText, 2 );
    FUNC( CheckGpkg, 0 );
    FUNC( InitGpkg, 0 );

    return SQLITE_OK;
}
