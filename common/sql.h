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
#ifndef GPKG_SQL_H
#define GPKG_SQL_H

#include <sqlite3.h>
#include "strbuf.h"

/**
 * \addtogroup sql SQL utilities
 * @{
 */

#define SQL_NOT_NULL_MASK 0x1
#define SQL_PRIMARY_KEY_MASK 0x2
#define SQL_UNIQUE_MASK 0x4

#define SQL_NOT_NULL SQL_NOT_NULL_MASK
#define SQL_PRIMARY_KEY SQL_PRIMARY_KEY_MASK
#define SQL_UNIQUE(i) (SQL_UNIQUE_MASK | (i << 4))
#define SQL_CONSTRAINT_IX(flags) (flags >> 4)
#define SQL_IS_CONSTRAINT(flags, mask, ix) ((flags & mask) && (SQL_CONSTRAINT_IX(flags) == ix))

typedef enum {
    VALUE_TEXT,
    VALUE_FUNC,
    VALUE_INTEGER,
    VALUE_DOUBLE,
    VALUE_NONE
} value_type_t;

typedef struct {
    union {
        char *text;
        double dbl;
        int integer;
    } value;
    value_type_t type;
} value_t;

#define NULL_VALUE {{NULL}, VALUE_NONE}
#define TEXT_VALUE(t) {{.text = t}, VALUE_TEXT}
#define FUNC_VALUE(t) {{.text = t}, VALUE_FUNC}
#define DOUBLE_VALUE(t) {{.dbl = t}, VALUE_DOUBLE}
#define INT_VALUE(t) {{.integer = t}, VALUE_INTEGER}

#define VALUE_AS_NULL(v) (NULL)
#define VALUE_AS_TEXT(v) (v.value.text)
#define VALUE_AS_FUNC(v) (v.value.text)
#define VALUE_AS_DOUBLE(v) (v.value.dbl)
#define VALUE_AS_INT(v) (v.value.integer)

typedef struct {
    char *name;
    char *type;
    value_t default_value;
    int flags;
    char *column_constraints;
} column_info_t;

typedef struct {
    char *name;
    column_info_t *columns;
    size_t nColumns;
    value_t *rows;
    size_t nRows;
} table_info_t;

int sql_begin(sqlite3 *db, char *name);

int sql_commit(sqlite3 *db, char *name);

int sql_rollback(sqlite3 *db, char *name);

int sql_exec(sqlite3 *db, char *sql, ...);

int sql_exec_for_string(sqlite3 *db, char **out, char *sql, ...);

int sql_exec_for_int(sqlite3 *db, int *out, char *sql, ...);

int sql_check_table_exists(sqlite3 *db, char* db_name, char* table_name, int *exists);

int sql_check_table(sqlite3 *db, char* db_name, table_info_t *table_info, int *errors, strbuf_t *errmsg);

int sql_init_table(sqlite3 *db, char* db_name, table_info_t *table_info, int *errors, strbuf_t *errmsg);

/** @} */

#endif
