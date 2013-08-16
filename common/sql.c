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
#include <stdarg.h>
#include <stdlib.h>
#include "sqlite.h"
#include "sql.h"

#define SQL_NOT_NULL_MASK SQL_NOT_NULL
#define SQL_AUTOINCREMENT_MASK SQL_AUTOINCREMENT
#define SQL_PRIMARY_KEY_MASK SQL_PRIMARY_KEY
#define SQL_UNIQUE_MASK SQL_UNIQUE(0)

static int sql_stmt_vinit(sqlite3_stmt **stmt, sqlite3 *db, char *sql, va_list args) {
    *stmt = NULL;
    char *formatted_sql = sqlite3_vmprintf(sql, args);

    if (formatted_sql == NULL) {
        return SQLITE_NOMEM;
    }

    int result = sqlite3_prepare_v2(db, formatted_sql, -1, stmt, NULL);
    sqlite3_free(formatted_sql);
    return result;
}

static int sql_stmt_init(sqlite3_stmt **stmt, sqlite3 *db, char *sql, ...) {
    va_list args;
    va_start(args, sql);
    int res = sql_stmt_vinit(stmt, db, sql, args);
    va_end(args);
    return res;
}

static int sql_stmt_bind(sqlite3_stmt *stmt, const value_t *values, const int nValues) {
    int result = sqlite3_reset(stmt);
    if (result != SQLITE_OK) {
        return result;
    }
    for (int cIx = 0; cIx < nValues; cIx++) {
        switch (values[cIx].type) {
            default:
                break;
            case VALUE_NULL:
                result = sqlite3_bind_null(stmt, cIx + 1);
                break;
            case VALUE_FUNC:
            case VALUE_TEXT:
                result = sqlite3_bind_text(stmt, cIx + 1, VALUE_AS_TEXT(values[cIx]), -1, SQLITE_STATIC);
                break;
            case VALUE_DOUBLE:
                result = sqlite3_bind_double(stmt, cIx + 1, VALUE_AS_DOUBLE(values[cIx]));
                break;
            case VALUE_INTEGER:
                result = sqlite3_bind_int(stmt, cIx + 1, VALUE_AS_INT(values[cIx]));
                break;
        }
        if (result != SQLITE_OK) {
            return result;
        }
    }

    return result;
}

static void sql_stmt_destroy(sqlite3_stmt *stmt) {
    if (stmt != NULL) {
        sqlite3_finalize(stmt);
    }
}

static int sql_step_for_string(sqlite3_stmt *stmt, char **out) {
    int result = SQLITE_OK;

    int stmt_res = sqlite3_step(stmt);
    if (stmt_res == SQLITE_ROW) {
        int col_count = sqlite3_column_count(stmt);
        if (col_count > 0) {
            int length = sqlite3_column_bytes(stmt, 0);
            if (length <= 0) {
                *out = NULL;
            } else {
                const unsigned char *text = sqlite3_column_text(stmt, 0);
                *out = sqlite3_malloc(length + 1);
                if (*out == NULL) {
                    result = SQLITE_NOMEM;
                    goto exit;
                }
                memmove(*out, text, (size_t)length + 1);
            }

        } else {
            *out = NULL;
        }
    } else if (stmt_res == SQLITE_DONE) {
        *out = NULL;
    } else {
        result = SQLITE_IOERR;
    }

    exit:
    return result;
}

static int sql_step_for_int(sqlite3_stmt *stmt, int *out) {
    int result = SQLITE_OK;

    int stmt_res = sqlite3_step(stmt);
    if (stmt_res == SQLITE_ROW) {
        int col_count = sqlite3_column_count(stmt);
        if (col_count > 0) {
            *out = sqlite3_column_int(stmt, 0);
        } else {
            *out = 0;
        }
    } else if (stmt_res == SQLITE_DONE) {
        *out = 0;
    } else {
        result = SQLITE_IOERR;
    }

    return result;
}

static int sql_step_for_double(sqlite3_stmt *stmt, double *out) {
    int result = SQLITE_OK;

    int stmt_res = sqlite3_step(stmt);
    if (stmt_res == SQLITE_ROW) {
        int col_count = sqlite3_column_count(stmt);
        if (col_count > 0) {
            *out = sqlite3_column_double(stmt, 0);
        } else {
            *out = 0;
        }
    } else if (stmt_res == SQLITE_DONE) {
        *out = 0;
    } else {
        result = SQLITE_IOERR;
    }

    return result;
}

int sql_exec_for_string(sqlite3 *db, char **out, char *sql, ...) {
    int result;
    sqlite3_stmt *stmt;

    va_list args;
    va_start(args, sql);
    result = sql_stmt_vinit(&stmt, db, sql, args);
    va_end(args);

    if (result != SQLITE_OK) {
        return result;
    }

    result = sql_step_for_string(stmt, out);

    sql_stmt_destroy(stmt);
    return result;
}

int sql_exec_for_int(sqlite3 *db, int *out, char *sql, ...) {
    int result;
    sqlite3_stmt *stmt;

    va_list args;
    va_start(args, sql);
    result = sql_stmt_vinit(&stmt, db, sql, args);
    va_end(args);

    if (result != SQLITE_OK) {
        return result;
    }

    result = sql_step_for_int(stmt, out);

    sql_stmt_destroy(stmt);
    return result;
}

int sql_exec_for_double(sqlite3 *db, double *out, char *sql, ...) {
    int result;
    sqlite3_stmt *stmt;

    va_list args;
    va_start(args, sql);
    result = sql_stmt_vinit(&stmt, db, sql, args);
    va_end(args);

    if (result != SQLITE_OK) {
        return result;
    }

    result = sql_step_for_double(stmt, out);

    sql_stmt_destroy(stmt);
    return result;
}

int sql_exec(sqlite3 *db, char *sql, ...) {
    int result;
    sqlite3_stmt *stmt;

    va_list args;
    va_start(args, sql);
    result = sql_stmt_vinit(&stmt, db, sql, args);
    va_end(args);

    if (result != SQLITE_OK) {
        return result;
    }

    int stmt_res = sqlite3_step(stmt);
    if (stmt_res != SQLITE_ROW && stmt_res != SQLITE_DONE) {
        result = stmt_res;
    }

    sql_stmt_destroy(stmt);
    return result;
}

int sql_exec_all(sqlite3 *db, char *sql, ...) {
    int result;
    sqlite3_stmt *stmt;

    va_list args;
    va_start(args, sql);
    result = sql_stmt_vinit(&stmt, db, sql, args);
    va_end(args);

    if (result != SQLITE_OK) {
        return result;
    }

    int stmt_res;
    do {
      stmt_res = sqlite3_step(stmt);
    } while (stmt_res == SQLITE_ROW);

    if (stmt_res != SQLITE_DONE) {
      result = stmt_res;
    }

    sql_stmt_destroy(stmt);
    return result;
}

int sql_check_table_exists(sqlite3 *db, const char* db_name, const char* table_name, int *exists) {
    sqlite3_stmt *stmt = NULL;
    int result = SQLITE_OK;

    result = sql_stmt_init(&stmt, db, "PRAGMA \"%w\".table_info(\"%w\")", db_name, table_name);
    if (result != SQLITE_OK) {
        return result;
    }

    result = sqlite3_step(stmt);
    if (result == SQLITE_ROW) {
        *exists = 1;
        result = SQLITE_OK;
    } else if (result == SQLITE_DONE) {
        *exists = 0;
        result = SQLITE_OK;
    } else {
        *exists = 0;
    }

    sql_stmt_destroy(stmt);
    return result;
}

static int sql_count_columns(const table_info_t *table_info) {
    int nColumns = 0;
    const column_info_t *column = table_info->columns;
    while(column->name != NULL) {
      column++;
      nColumns++;
    }
    return nColumns;
}

static int sql_check_cols(sqlite3_stmt *stmt, const table_info_t *table_info, error_t *error) {
    int nColumns = sql_count_columns(table_info);
    int found[nColumns];
    memset(found, 0, nColumns * sizeof(int));
    int result;

    result = sqlite3_step(stmt);

    while( result == SQLITE_ROW ) {
        // 0 index
        // 1 name
        // 2 type
        // 3 not null
        // 4 default value
        // 5 primary key
        char *name = (char*)sqlite3_column_text(stmt, 1);
        int index = -1;
        for( int c = 0; c < nColumns; c++) {
            if (sqlite3_stricmp(table_info->columns[c].name, name) == 0) {
                index = c;
                break;
            }
        }

        if (index != -1) {
            char *type = (char*)sqlite3_column_text(stmt, 2);
            if (sqlite3_stricmp(table_info->columns[index].type, type) != 0) {
                if (error) {
                    error_append(error, "Column %s.%s has incorrect type (expected: %s, actual: %s)\n", table_info->name, name, table_info->columns[index].type, type);
                }
            }

            int not_null = sqlite3_column_int(stmt, 3);
            if (not_null != 0 && (table_info->columns[index].flags & SQL_NOT_NULL) == 0) {
                if (error) {
                    error_append(error, "Column %s.%s should not have 'not null' constraint\n", table_info->name, name);
                }
            } else if (not_null == 0 && table_info->columns[index].flags & SQL_NOT_NULL) {
                if (error) {
                    error_append(error, "Column %s.%s should have 'not null' constraint\n", table_info->name, name);
                }
            }

            int pk = sqlite3_column_int(stmt, 5);
            if (pk != 0 && (table_info->columns[index].flags & SQL_PRIMARY_KEY) == 0) {
                if (error) {
                    error_append(error, "Column %s.%s should not be part of primary key\n", table_info->name, name);
                }
            } else if (pk == 0 && table_info->columns[index].flags & SQL_PRIMARY_KEY) {
                if (error) {
                    error_append(error, "Column %s.%s should be part of primary key\n", table_info->name, name);
                }
            }
            found[index] = 1;
        } else {
            if (error) {
                error_append(error, "Redundant column %s.%s\n", table_info->name, name);
            }
        }

        result = sqlite3_step(stmt);
    }

    if (result != SQLITE_DONE) {
        return result;
    }

    for (int i = 0; i < nColumns; i++) {
        if (found[i] == 0) {
            if (error) {
                error_append(error, "Column %s.%s is missing\n", table_info->name, table_info->columns[i].name);
            }
        }
    }
    return SQLITE_OK;
}

static int sql_check_table_schema(sqlite3 *db, const char* db_name, const table_info_t* table_info, error_t *error) {
    sqlite3_stmt *stmt = NULL;
    int result = sql_stmt_init(&stmt, db, "PRAGMA \"%w\".table_info(\"%w\")", db_name, table_info->name);
    if (result == SQLITE_OK) {
        result = sql_check_cols(stmt, table_info, error);
    }
    sql_stmt_destroy(stmt);
    return result;
}

static int sql_format_missing_row(const char* db_name, const table_info_t *table_info, const value_t *row, error_t *error) {
    int result;
    strbuf_t errmsg;

    result = strbuf_init(&errmsg, 256);
    if (result != SQLITE_OK) {
        goto exit;
    }

    result = strbuf_append(&errmsg, "Table %s.%s is missing row (", db_name, table_info->name);
    if (result != SQLITE_OK) {
        goto exit;
    }

    int nColumns = sql_count_columns(table_info);
    for (int cIx = 0; cIx < nColumns; cIx++) {
        if (cIx > 0) {
            result = strbuf_append(&errmsg, ", ");
            if (result != SQLITE_OK) {
                goto exit;
            }
        }

        switch (row[cIx].type) {
            default:
                break;
            case VALUE_NULL:
                result = strbuf_append(&errmsg, "%s: NULL", table_info->columns[cIx].name);
                break;
            case VALUE_FUNC:
            case VALUE_TEXT:
                result = strbuf_append(&errmsg, "%s: '%s'", table_info->columns[cIx].name, VALUE_AS_TEXT(row[cIx]));
                break;
            case VALUE_DOUBLE:
                result = strbuf_append(&errmsg, "%s: %g", table_info->columns[cIx].name, VALUE_AS_DOUBLE(row[cIx]));
                break;
            case VALUE_INTEGER:
                result = strbuf_append(&errmsg, "%s: %d", table_info->columns[cIx].name, VALUE_AS_INT(row[cIx]));
                break;
        }
        if (result != SQLITE_OK) {
            goto exit;
        }
    }
    result = strbuf_append(&errmsg, ")", table_info->name);
    if (result != SQLITE_OK) {
        goto exit;
    }

    result = error_append(error, "%s", strbuf_data_pointer(&errmsg));

    exit:
    strbuf_destroy(&errmsg);
    return result;
}

static int sql_check_data(sqlite3 *db, const char* db_name, const table_info_t* table_info, error_t *error) {
    if (table_info->nRows <= 0) {
        return SQLITE_OK;
    }

    int nColumns = sql_count_columns(table_info);
    
    int result = SQLITE_OK;
    strbuf_t sql;
    char *query = NULL;
    sqlite3_stmt *stmt = NULL;

    result = strbuf_init(&sql, 4096);
    if (result != SQLITE_OK) {
        goto exit;
    }

    const column_info_t *columns = table_info->columns;
    strbuf_append(&sql, "SELECT * FROM \"%w\".\"%w\" WHERE", db_name, table_info->name);
    for(int i = 0; i < nColumns; i++) {
        if (i > 0) {
            strbuf_append(&sql, " AND");
        }
        strbuf_append(&sql, " \"%w\" IS ?", columns[i].name);
    }
    result = strbuf_data(&sql, &query);

    if (result != SQLITE_OK) {
        goto exit;
    }

    result = sql_stmt_init(&stmt, db, query);
    if (result != SQLITE_OK) {
        goto exit;
    }

    for (int rIx = 0; rIx < table_info->nRows; rIx++) {
        const value_t *row = table_info->rows + (rIx * nColumns);
        result = sql_stmt_bind(stmt, row, nColumns);
        if (result != SQLITE_OK) {
            goto exit;
        }

        int step_res = sqlite3_step(stmt);
        if (step_res == SQLITE_ROW) {
            // OK
        } else if (step_res == SQLITE_DONE) {
            if (error) {
                result = sql_format_missing_row(db_name, table_info, row, error);
                if (result != SQLITE_OK) {
                    goto exit;
                }
            }
        } else {
            result = step_res;
            goto exit;
        }
    }

    exit:
    strbuf_destroy(&sql);
    sql_stmt_destroy(stmt);
    sqlite3_free(query);
    return result;
}

static int sql_format_insert_data(const char *db_name, const table_info_t* table_info, char **query) {
    strbuf_t sql;
    int result = strbuf_init(&sql, 4096);
    if (result != SQLITE_OK) {
        return result;
    }

    int nColumns = sql_count_columns(table_info);
    const column_info_t *columns = table_info->columns;
    result = strbuf_append(&sql, "INSERT OR IGNORE INTO \"%w\".\"%w\" (", db_name, table_info->name);
    if (result != SQLITE_OK) {
        goto exit;
    }
    for(int i = 0; i < nColumns; i++) {
        if (i > 0) {
            result = strbuf_append(&sql, ",\"%w\"", columns[i].name);
        } else {
            result = strbuf_append(&sql, "\"%w\"", columns[i].name);
        }
        if (result != SQLITE_OK) {
            goto exit;
        }
    }
    result = strbuf_append(&sql, ") VALUES (", table_info->name);
    if (result != SQLITE_OK) {
        goto exit;
    }

    for(int i = 0; i < nColumns; i++) {
        if (i > 0) {
            result = strbuf_append(&sql, ",?");
        } else {
            result = strbuf_append(&sql, "?");
        }
        if (result != SQLITE_OK) {
            goto exit;
        }
    }
    result = strbuf_append(&sql, ")");

    if (result == SQLITE_OK) {
        result = strbuf_data(&sql, query);
    }

    exit:
    strbuf_destroy(&sql);
    return result;
}

static int sql_insert_data(sqlite3 *db, const char *db_name, const table_info_t* table_info, error_t *error) {
    if (table_info->nRows <= 0) {
        return SQLITE_OK;
    }

    sqlite3_stmt *stmt = NULL;
    int result = SQLITE_OK;
    char* query = NULL;

    result = sql_format_insert_data(db_name, table_info, &query);
    if (result != SQLITE_OK) {
        goto exit;
    }

    result = sql_stmt_init(&stmt, db, query);
    if (result != SQLITE_OK) {
        goto exit;
    }

    int nColumns = sql_count_columns(table_info);

    for (int rIx = 0; rIx < table_info->nRows; rIx++) {
        const value_t *row = table_info->rows + (rIx * nColumns);
        result = sql_stmt_bind(stmt, row, nColumns);
        if (result != SQLITE_OK) {
            goto exit;
        }
        int step_res = sqlite3_step(stmt);
        if (step_res != SQLITE_DONE) {
            result = step_res;
            if (error) {
                result = error_append(error, sqlite3_errmsg(db));
                if (result != SQLITE_OK) {
                    goto exit;
                }
            }
            goto exit;
        }
    }

    exit:
    sqlite3_free(query);
    sql_stmt_destroy(stmt);
    return result;
}

#define SQL_CONSTRAINT_IX(flags) ( (flags >> 4) & 0xF)
#define SQL_IS_CONSTRAINT(flags, mask, ix) ((flags & mask) && (SQL_CONSTRAINT_IX(flags) == ix))

static void appendTableConstraint(const table_info_t *table_info, strbuf_t *sql, int constraint_mask, int constraint_idx) {
    char* constraint_name;
    if (constraint_mask == SQL_PRIMARY_KEY_MASK) {
        constraint_name = "PRIMARY KEY";
    } else if (constraint_mask == SQL_UNIQUE_MASK) {
        constraint_name = "UNIQUE";
    } else {
        return;
    }

    int has_cols = 0;
    int nColumns = sql_count_columns(table_info);

    for(int i = 0; i < nColumns; i++) {
        int flags = table_info->columns[i].flags;
        if (SQL_IS_CONSTRAINT(flags, constraint_mask, constraint_idx)) {
            has_cols = 1;
            break;
        }
    }

    if (!has_cols) {
        return;
    }

    strbuf_append(sql, ",\n  %s (", constraint_name);
    int first = 1;
    for(int i = 0; i < nColumns; i++) {
        int flags = table_info->columns[i].flags;
        if (SQL_IS_CONSTRAINT(flags, constraint_mask, constraint_idx)) {
            if (first) {
                first = 0;
                strbuf_append(sql, "\"%w\"", table_info->columns[i].name);
            } else {
                strbuf_append(sql, ", \"%w\"", table_info->columns[i].name);
            }
        }
    }
    strbuf_append(sql, ")");
}

static int sql_create_table(sqlite3 *db, const char *db_name, const table_info_t* table_info, error_t *error) {
    int result;
    strbuf_t sql;
    result = strbuf_init(&sql, 4096);
    if (result != SQLITE_OK) {
        return result;
    }

    int pk_count = 0;
    int max_uk = -1;
    int nColumns = sql_count_columns(table_info);

    const column_info_t *columns = table_info->columns;
    for(int i = 0; i < nColumns; i++) {
        if (columns[i].flags & SQL_PRIMARY_KEY_MASK) {
            pk_count++;
        }
    }

    strbuf_append(&sql, "CREATE TABLE IF NOT EXISTS \"%w\".\"%w\" (", db_name, table_info->name);
    for(int i = 0; i < nColumns; i++) {
        if (i > 0) {
            strbuf_append(&sql, ",\n  \"%w\" %s", columns[i].name, columns[i].type);
        } else {
            strbuf_append(&sql, "\n  \"%w\" %s", columns[i].name, columns[i].type);
        }
        int flags = columns[i].flags;
        if (flags & SQL_NOT_NULL_MASK) {
            strbuf_append(&sql, " NOT NULL");
        }
        if ((flags & SQL_PRIMARY_KEY_MASK) && pk_count == 1) {
            strbuf_append(&sql, " PRIMARY KEY");
            if (flags & SQL_AUTOINCREMENT) {
                strbuf_append(&sql, " AUTOINCREMENT");
            }
        }

        switch (columns[i].default_value.type) {
            default:
                break;
            case VALUE_TEXT:
                strbuf_append(&sql, " DEFAULT %Q", VALUE_AS_TEXT(columns[i].default_value));
                break;
            case VALUE_FUNC:
                strbuf_append(&sql, " DEFAULT (%s)", VALUE_AS_FUNC(columns[i].default_value));
                break;
            case VALUE_DOUBLE:
                strbuf_append(&sql, " DEFAULT %g", VALUE_AS_DOUBLE(columns[i].default_value));
                break;
            case VALUE_INTEGER:
                strbuf_append(&sql, " DEFAULT %d", VALUE_AS_INT(columns[i].default_value));
                break;
        }

        if (columns[i].column_constraints != NULL) {
            strbuf_append(&sql, " %s", columns[i].column_constraints);
        }

        if (flags & SQL_UNIQUE_MASK) {
            int ix = SQL_CONSTRAINT_IX(flags);
            if (ix > max_uk) {
                max_uk = ix;
            }
        }
    }

    if (pk_count > 1) {
        appendTableConstraint(table_info, &sql, SQL_PRIMARY_KEY_MASK, 0);
    }

    if (max_uk > 0) {
        for( int i = 0; i <= max_uk; i++) {
            appendTableConstraint(table_info, &sql, SQL_UNIQUE_MASK, i);
        }
    }

    strbuf_append(&sql, "\n)");

    result = sql_exec(db, strbuf_data_pointer(&sql));
    if (result != SQLITE_OK && error) {
        error_append(error, sqlite3_errmsg(db));
    }

    strbuf_destroy(&sql);

    return result;
}

static int sql_init_check_table(sqlite3* db, const char* db_name, const table_info_t* table_info, int create, error_t *error) {
    if (error == NULL) {
        return SQLITE_MISUSE;
    }

    int exists = 0;
    int result = sql_check_table_exists(db, db_name, table_info->name, &exists);
    if (result == SQLITE_OK) {
        if (exists) {
            result = sql_check_table_schema(db, db_name, table_info, error);

            if (result == SQLITE_OK && create != 0) {
                result = sql_insert_data(db, db_name, table_info, error);
            }

            if (result == SQLITE_OK) {
                result = sql_check_data(db, db_name, table_info, error);
            }
        } else {
            if (create == 0) {
                error_append(error, "Table %s.%s does not exist", db_name, table_info->name);
            } else {
                result = sql_create_table(db, db_name, table_info, error);

                if (result == SQLITE_OK) {
                    result = sql_insert_data(db, db_name, table_info, error);
                }
            }
        }
    }

    return result;
}

int sql_check_table(sqlite3* db, const char* db_name, const table_info_t* table_info, error_t *error) {
    return sql_init_check_table(db, db_name, table_info, 0, error);
}

int sql_init_table(sqlite3 *db, const char* db_name, const table_info_t* table_info, error_t *error) {
    return sql_init_check_table(db, db_name, table_info, 1, error);
}

int sql_begin(sqlite3 *db, char *name) {
    return sql_exec(db, "SAVEPOINT %Q", name);
}

int sql_commit(sqlite3 *db, char *name) {
    return sql_exec(db, "RELEASE SAVEPOINT %Q", name);
}

int sql_rollback(sqlite3 *db, char *name) {
    return sql_exec(db, "ROLLBACK TO SAVEPOINT %Q", name);
}
