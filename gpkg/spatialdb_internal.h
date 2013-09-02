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

#define FUNCTION_END_INT_RESULT(context, sqlite_result) \
  exit:\
    if (FUNCTION_RESULT == SQLITE_OK) {\
        if (error_count(FUNCTION_ERROR_PTR) > 0) {\
            sqlite3_result_error(context, error_message(FUNCTION_ERROR_PTR), -1);\
        } else {\
            sqlite3_result_int(context, sqlite_result);\
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

#endif