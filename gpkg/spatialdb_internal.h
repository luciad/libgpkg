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
#ifndef GPKG_SPATIALDB_INTERNAL_H
#define GPKG_SPATIALDB_INTERNAL_H

#include "spatialdb.h"

#define FUNCTION_NOOP do {} while(0)
#define FUNCTION_RESULT _result
#define FUNCTION_DB_HANDLE _db_handle
#define FUNCTION_ERROR _error_ptr

#define FUNCTION_START(context)                                                                                        \
    int FUNCTION_RESULT = SQLITE_OK;                                                                                   \
    errorstream_t _error;                                                                                                    \
    errorstream_t* FUNCTION_ERROR = &_error;                                                                                 \
    if (error_init(FUNCTION_ERROR) != SQLITE_OK) {                                                                     \
        sqlite3_result_error(context, "Could not init error buffer", -1);                                              \
        goto exit;                                                                                                     \
    };                                                                                                                 \
    sqlite3 *FUNCTION_DB_HANDLE = sqlite3_context_db_handle(context)

#define FUNCTION_START_STATIC(context, error_buf_size)                                                                 \
    int FUNCTION_RESULT = SQLITE_OK;                                                                                   \
    char error_buffer[error_buf_size];                                                                                 \
    errorstream_t _error;                                                                                                    \
    errorstream_t* FUNCTION_ERROR = &_error;                                                                                 \
    if (error_init_fixed(FUNCTION_ERROR, error_buffer, error_buf_size) != SQLITE_OK) {                                 \
        sqlite3_result_error(context, "Could not init error buffer", -1);                                              \
        goto exit;                                                                                                     \
    }                                                                                                                  \
    sqlite3 *FUNCTION_DB_HANDLE = sqlite3_context_db_handle(context)

#define FUNCTION_START_NESTED(context, error)                                                                          \
    int FUNCTION_RESULT = SQLITE_OK;                                                                                   \
    errorstream_t* FUNCTION_ERROR = error;                                                                                   \
    sqlite3 *FUNCTION_DB_HANDLE = sqlite3_context_db_handle(context)

#define FUNCTION_END(context)                                                                                          \
  exit:                                                                                                                \
    if (FUNCTION_RESULT == SQLITE_OK) {                                                                                \
        if (error_count(FUNCTION_ERROR) > 0) {                                                                         \
            if (strlen(error_message(FUNCTION_ERROR)) == 0) {                                                          \
              error_append(FUNCTION_ERROR, "unknown error" );                                                          \
            }                                                                                                          \
            sqlite3_result_error(context, error_message(FUNCTION_ERROR), -1);                                          \
        }                                                                                                              \
    } else {                                                                                                           \
      if (error_count(FUNCTION_ERROR) == 0 || strlen(error_message(FUNCTION_ERROR)) == 0) {                            \
        error_append(FUNCTION_ERROR, "unknown error: %d", FUNCTION_RESULT );                                           \
      }                                                                                                                \
      sqlite3_result_error(context, error_message(FUNCTION_ERROR), -1);                                                \
    }                                                                                                                  \
    error_destroy(FUNCTION_ERROR)

#define FUNCTION_END_NESTED(context)                                                                                   \
  exit:                                                                                                                \
    FUNCTION_NOOP


#define FUNCTION_START_TRANSACTION(name)                                                                               \
    char *name##_transaction = #name;                                                                                  \
    do {                                                                                                               \
      FUNCTION_RESULT = sql_begin(FUNCTION_DB_HANDLE, name##_transaction);                                             \
      if (FUNCTION_RESULT != SQLITE_OK) {                                                                              \
        goto exit;                                                                                                     \
      }                                                                                                                \
    } while(0)
#define FUNCTION_END_TRANSACTION(name) do {                                                                            \
        if (FUNCTION_RESULT == SQLITE_OK && error_count(FUNCTION_ERROR) == 0) {                                        \
            FUNCTION_RESULT = sql_commit(FUNCTION_DB_HANDLE, name##_transaction);                                      \
        } else {                                                                                                       \
            sql_rollback(FUNCTION_DB_HANDLE, name##_transaction);                                                      \
        }                                                                                                              \
    } while(0)

#define FUNCTION_INT_ARG(arg) int32_t arg = 0
#define FUNCTION_GET_INT_ARG(arg, ix) arg = sqlite3_value_int(args[ix])
#define FUNCTION_SET_INT_ARG(arg, val) arg = val
#define FUNCTION_FREE_INT_ARG(arg) FUNCTION_NOOP

#define FUNCTION_TEXT_ARG_LENGTH(arg) arg##length
#define FUNCTION_TEXT_ARG(arg)                                                                                         \
    const char* arg = NULL;                                                                                            \
    size_t FUNCTION_TEXT_ARG_LENGTH(arg);                                                                              \
    int free_##arg = 0
#define FUNCTION_GET_TEXT_ARG(context, arg, ix)                                                                        \
    arg = (const char *)sqlite3_value_text(args[ix]);                                                                  \
    FUNCTION_TEXT_ARG_LENGTH(arg) = (size_t) sqlite3_value_bytes(args[ix]);                                            \
    do {                                                                                                               \
        if (arg != NULL) {                                                                                             \
          arg = sqlite3_mprintf("%s", sqlite3_value_text(args[ix]));                                                   \
          free_##arg = 1;                                                                                              \
          if (arg == NULL) {                                                                                           \
            sqlite3_result_error_code(context, SQLITE_NOMEM);                                                          \
            goto exit;                                                                                                 \
          }                                                                                                            \
        }                                                                                                              \
    } while(0);
#define FUNCTION_GET_TEXT_ARG_UNSAFE(arg,ix)                                                                           \
    arg = (const char *)sqlite3_value_text(args[ix]);                                                                  \
    FUNCTION_TEXT_ARG_LENGTH(arg) = (size_t) sqlite3_value_bytes(args[ix])
#define FUNCTION_SET_TEXT_ARG(arg, val)                                                                                \
    arg = val;                                                                                                         \
    free_##arg = 0
#define FUNCTION_FREE_TEXT_ARG(arg)                                                                                    \
    do {                                                                                                               \
        if (free_##arg != 0) {                                                                                         \
            sqlite3_free((void*)arg);                                                                                  \
            arg = NULL;                                                                                                \
        }                                                                                                              \
    } while (0)
#define FUNCTION_BLOB_ARG(arg)                                                                                         \
    uint8_t* arg = NULL;                                                                                               \
    size_t arg##_length
#define FUNCTION_GET_BLOB_ARG_UNSAFE(context,arg,ix)                                                                   \
    arg = (uint8_t *)sqlite3_value_blob(args[ix]);                                                                     \
    arg##_length = (size_t) sqlite3_value_bytes(args[ix]);                                                             \
    do {                                                                                                               \
        if (arg == NULL || arg##_length == 0) {                                                                        \
            sqlite3_result_null(context);                                                                              \
            goto exit;                                                                                                 \
        }                                                                                                              \
    } while (0)
#define FUNCTION_FREE_BLOB_ARG(arg) FUNCTION_NOOP

#define FUNCTION_STREAM_ARG(arg)                                                                                       \
    FUNCTION_BLOB_ARG(arg##_blob);                                                                                     \
    binstream_t arg
#define FUNCTION_GET_STREAM_ARG_UNSAFE(context, arg,ix)                                                                \
    FUNCTION_GET_BLOB_ARG_UNSAFE(context, arg##_blob,ix);                                                              \
    binstream_init(&arg, arg##_blob, arg##_blob_length)
#define FUNCTION_FREE_STREAM_ARG(arg)                                                                                  \
    FUNCTION_FREE_BLOB_ARG(arg##_blob);                                                                                \
    binstream_destroy(&arg, 0)

#define FUNCTION_GEOM_ARG_STREAM(arg) arg##_stream
#define FUNCTION_GEOM_ARG(arg)                                                                                         \
    FUNCTION_STREAM_ARG( arg##_stream );                                                                               \
    geom_blob_header_t arg
#define FUNCTION_GET_GEOM_ARG_UNSAFE(context, spatialdb, arg,ix)                                                       \
    FUNCTION_GET_STREAM_ARG_UNSAFE(context, arg##_stream,ix);                                                          \
    do {                                                                                                               \
        if (spatialdb->read_blob_header(&arg##_stream, &arg, FUNCTION_ERROR) != SQLITE_OK) {                           \
            if ( error_count(FUNCTION_ERROR) == 0 ) {                                                                  \
                error_append(FUNCTION_ERROR, "Invalid geometry blob header");                                          \
            }                                                                                                          \
            goto exit;                                                                                                 \
        }                                                                                                              \
    } while (0)

#define FUNCTION_FREE_GEOM_ARG(arg) FUNCTION_FREE_STREAM_ARG(arg##_stream)

#define FUNCTION_WKB_ARG_GEOM(arg) arg##_geom
#define FUNCTION_WKB_ARG(arg)                                                                                          \
    FUNCTION_GEOM_ARG(arg##_geom);                                                                                     \
    geom_header_t arg
#define FUNCTION_GET_WKB_ARG_UNSAFE(context, spatialdb, arg,ix)                                                        \
    FUNCTION_GET_GEOM_ARG_UNSAFE(context, spatialdb, arg##_geom,ix);                                                   \
    do {                                                                                                               \
        if (spatialdb->read_geometry_header(&arg##_geom_stream, &arg, FUNCTION_ERROR) != SQLITE_OK) {                  \
            if ( error_count(FUNCTION_ERROR) == 0 ) {                                                                  \
                error_append(FUNCTION_ERROR, "Invalid geometry blob header");                                          \
            }                                                                                                          \
            goto exit;                                                                                                 \
        }                                                                                                              \
    } while (0)

#define FUNCTION_FREE_WKB_ARG(arg) FUNCTION_FREE_GEOM_ARG(arg##_geom)

#define FUNCTION_GET_TYPE(arg, ix) arg = sqlite3_value_type(args[ix])

#endif
