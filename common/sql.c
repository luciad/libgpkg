#include <stdarg.h>
#include <stdlib.h>
#include "str.h"
#include "sqlite.h"
#include "sql.h"

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

static int sql_stmt_bind(sqlite3_stmt *stmt, value_t *values, int nValues) {
    int result = sqlite3_reset(stmt);
    if (result != SQLITE_OK) {
        return result;
    }
    for (int cIx = 0; cIx < nValues; cIx++) {
        switch (values[cIx].type) {
            default:
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

int sql_check_table_exists(sqlite3 *db, char* db_name, char* table_name, int *exists) {
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

static int sql_check_cols(sqlite3_stmt *stmt, table_info_t *table_info, int *errors, strbuf_t *errmsg) {
    int found[table_info->nColumns];
    memset(found, 0, table_info->nColumns * sizeof(int));
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
        for( int c = 0; c < table_info->nColumns; c++) {
            if (STRICMP(table_info->columns[c].name, name) == 0) {
                index = c;
                break;
            }
        }

        if (index != -1) {
            char *type = (char*)sqlite3_column_text(stmt, 2);
            if (STRICMP(table_info->columns[index].type, type) != 0) {
                *errors = *errors + 1;
                if (errmsg) {
                    strbuf_append(errmsg, "Column %s.%s has incorrect type (expected: %s, actual: %s)\n", table_info->name, name, table_info->columns[index].type, type);
                }
            }

            int not_null = sqlite3_column_int(stmt, 3);
            if (not_null != 0 && (table_info->columns[index].flags & SQL_NOT_NULL) == 0) {
                *errors = *errors + 1;
                if (errmsg) {
                    strbuf_append(errmsg, "Column %s.%s should not have 'not null' constraint\n", table_info->name, name);
                }
            } else if (not_null == 0 && table_info->columns[index].flags & SQL_NOT_NULL) {
                *errors = *errors + 1;
                if (errmsg) {
                    strbuf_append(errmsg, "Column %s.%s should have 'not null' constraint\n", table_info->name, name);
                }
            }

            int pk = sqlite3_column_int(stmt, 5);
            if (pk != 0 && (table_info->columns[index].flags & SQL_PRIMARY_KEY) == 0) {
                *errors = *errors + 1;
                if (errmsg) {
                    strbuf_append(errmsg, "Column %s.%s should not be part of primary key\n", table_info->name, name);
                }
            } else if (pk == 0 && table_info->columns[index].flags & SQL_PRIMARY_KEY) {
                *errors = *errors + 1;
                if (errmsg) {
                    strbuf_append(errmsg, "Column %s.%s should be part of primary key\n", table_info->name, name);
                }
            }
            found[index] = 1;
        } else {
            *errors = *errors + 1;
            if (errmsg) {
                strbuf_append(errmsg, "Redundant column %s.%s\n", table_info->name, name);
            }
        }

        result = sqlite3_step(stmt);
    }

    if (result != SQLITE_DONE) {
        return result;
    }

    for (int i = 0; i < table_info->nColumns; i++) {
        if (found[i] == 0) {
            *errors = *errors + 1;
            if (errmsg) {
                strbuf_append(errmsg, "Column %s.%s is missing\n", table_info->name, table_info->columns[i].name);
            }
        }
    }
    return SQLITE_OK;
}

static int sql_check_table_schema(sqlite3 *db, char* db_name, table_info_t* table_info, int *errors, strbuf_t *errmsg) {
    sqlite3_stmt *stmt = NULL;
    int result = sql_stmt_init(&stmt, db, "PRAGMA \"%w\".table_info(\"%w\")", db_name, table_info->name);
    if (result == SQLITE_OK) {
        result = sql_check_cols(stmt, table_info, errors, errmsg);
    }
    sql_stmt_destroy(stmt);
    return result;
}

static int sql_format_missing_row(char* db_name, table_info_t *table_info, strbuf_t *errmsg, value_t *row) {
    int result;

    result = strbuf_append(errmsg, "Table %s.%s is missing row (", db_name, table_info->name);
    if (result != SQLITE_OK) {
        goto exit;
    }
    for (int cIx = 0; cIx < table_info->nColumns; cIx++) {
        if (cIx > 0) {
            result = strbuf_append(errmsg, ", ");
            if (result != SQLITE_OK) {
                goto exit;
            }
        }

        switch (row[cIx].type) {
            default:
                break;
            case VALUE_FUNC:
            case VALUE_TEXT:
                result = strbuf_append(errmsg, "%s: '%s'", table_info->columns[cIx].name, VALUE_AS_TEXT(row[cIx]));
                break;
            case VALUE_DOUBLE:
                result = strbuf_append(errmsg, "%s: %g", table_info->columns[cIx].name, VALUE_AS_DOUBLE(row[cIx]));
                break;
            case VALUE_INTEGER:
                result = strbuf_append(errmsg, "%s: %d", table_info->columns[cIx].name, VALUE_AS_INT(row[cIx]));
                break;
        }
        if (result != SQLITE_OK) {
            goto exit;
        }
    }
    result = strbuf_append(errmsg, ")\n", table_info->name);

    exit:
    return result;
}

static int sql_check_data(sqlite3 *db, char* db_name, table_info_t* table_info, int *errors, strbuf_t *errmsg) {
    if (table_info->nRows <= 0 || table_info->nColumns <= 0) {
        return SQLITE_OK;
    }
    
    int result = SQLITE_OK;
    strbuf_t sql;
    char *query = NULL;

    result = strbuf_init(&sql, 4096);
    if (result != SQLITE_OK) {
        goto exit;
    }

    column_info_t *columns = table_info->columns;
    strbuf_append(&sql, "SELECT * FROM \"%w\".\"%w\" WHERE", db_name, table_info->name);
    for(int i = 0; i < table_info->nColumns; i++) {
        if (i > 0) {
            strbuf_append(&sql, " AND");
        }
        strbuf_append(&sql, " \"%w\" = ?", columns[i].name);
    }
    result = strbuf_data(&sql, &query);

    if (result != SQLITE_OK) {
        goto exit;
    }

    sqlite3_stmt *stmt = NULL;
    result = sql_stmt_init(&stmt, db, query);
    if (result != SQLITE_OK) {
        goto exit;
    }

    for (int rIx = 0; rIx < table_info->nRows; rIx++) {
        value_t *row = table_info->rows + (rIx * table_info->nColumns);
        result = sql_stmt_bind(stmt, row, table_info->nColumns);
        if (result != SQLITE_OK) {
            goto exit;
        }

        int step_res = sqlite3_step(stmt);
        if (step_res == SQLITE_ROW) {
            // OK
        } else if (step_res == SQLITE_DONE) {
            *errors = *errors + 1;
            if (errmsg) {
                result = sql_format_missing_row(db_name, table_info, errmsg, row);
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

int sql_check_table(sqlite3* db, char* db_name, table_info_t* table_info, int *errors, strbuf_t* errmsg) {
    if (errors == NULL) {
        return SQLITE_MISUSE;
    }

    int exists = 0;
    int result = sql_check_table_exists(db, db_name, table_info->name, &exists);
    if (result == SQLITE_OK) {
        if (exists) {
            if (result == SQLITE_OK) {
                result = sql_check_table_schema(db, db_name, table_info, errors, errmsg);
            }

            if (result == SQLITE_OK) {
                result = sql_check_data(db, db_name, table_info, errors, errmsg);
            }
        } else {
            *errors = *errors + 1;
            if (errmsg) {
                strbuf_append(errmsg, "Table %s.%s does not exist\n", db_name, table_info->name);
            }
        }
    }

    return result;
}

static int sql_format_insert_data(char *db_name, table_info_t* table_info, char **query) {
    strbuf_t sql;
    int result = strbuf_init(&sql, 4096);
    if (result != SQLITE_OK) {
        return result;
    }

    column_info_t *columns = table_info->columns;
    result = strbuf_append(&sql, "INSERT OR REPLACE INTO \"%w\".\"%w\" (", db_name, table_info->name);
    if (result != SQLITE_OK) {
        goto exit;
    }
    for(int i = 0; i < table_info->nColumns; i++) {
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

    for(int i = 0; i < table_info->nColumns; i++) {
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

static int sql_insert_data(sqlite3 *db, char *db_name, table_info_t* table_info, int *errors, strbuf_t *errmsg) {
    if (table_info->nRows <= 0 || table_info->nColumns <= 0) {
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

    for (int rIx = 0; rIx < table_info->nRows; rIx++) {
        value_t *row = table_info->rows + (rIx * table_info->nColumns);
        result = sql_stmt_bind(stmt, row, table_info->nColumns);
        if (result != SQLITE_OK) {
            goto exit;
        }
        int step_res = sqlite3_step(stmt);
        if (step_res != SQLITE_DONE) {
            result = step_res;
            *errors = *errors + 1;
            if (errmsg) {
                result = strbuf_append(errmsg, sqlite3_errmsg(db));
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

static void appendTableConstraint(table_info_t *table_info, strbuf_t *sql, int constraint_mask, int constraint_idx) {
    char* constraint_name;
    if (constraint_mask == SQL_PRIMARY_KEY_MASK) {
        constraint_name = "PRIMARY KEY";
    } else if (constraint_mask == SQL_UNIQUE_MASK) {
        constraint_name = "UNIQUE KEY";
    } else {
        return;
    }

    int has_cols = 0;
    for(int i = 0; i < table_info->nColumns; i++) {
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
    for(int i = 0; i < table_info->nColumns; i++) {
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

static int sql_create_table(sqlite3 *db, char *db_name, table_info_t* table_info, int *errors, strbuf_t *errmsg) {
    int result;
    strbuf_t sql;
    result = strbuf_init(&sql, 4096);
    if (result != SQLITE_OK) {
        return result;
    }

    int has_pk = 0;
    int max_uk = -1;

    strbuf_append(&sql, "CREATE TABLE IF NOT EXISTS \"%w\".\"%w\" (", db_name, table_info->name);
    column_info_t *columns = table_info->columns;
    for(int i = 0; i < table_info->nColumns; i++) {
        if (i > 0) {
            strbuf_append(&sql, ",\n  \"%w\" %s", columns[i].name, columns[i].type);
        } else {
            strbuf_append(&sql, "\n  \"%w\" %s", columns[i].name, columns[i].type);
        }
        int flags = columns[i].flags;
        if (flags & SQL_NOT_NULL_MASK) {
            strbuf_append(&sql, " NOT NULL");
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

        if (flags & SQL_PRIMARY_KEY_MASK) {
            has_pk = 1;
        }

        if (flags & SQL_UNIQUE_MASK) {
            int ix = SQL_CONSTRAINT_IX(flags);
            if (ix > max_uk) {
                max_uk = ix;
            }
        }
    }

    if (has_pk) {
        appendTableConstraint(table_info, &sql, SQL_PRIMARY_KEY_MASK, 0);
    }

    if (max_uk > 0) {
        for( int i = 0; i <= max_uk; i++) {
            appendTableConstraint(table_info, &sql, SQL_UNIQUE_MASK, i);
        }
    }

    strbuf_append(&sql, "\n)");

    char* err = NULL;
    result = sqlite3_exec(db, strbuf_data_pointer(&sql), NULL, NULL, &err);
    if (err) {
        *errors = *errors + 1;
        if (errmsg) {
            strbuf_append(errmsg, err);
        }
    }

    strbuf_destroy(&sql);

    return result;
}

int sql_init_table(sqlite3 *db, char* db_name, table_info_t* table_info, int *errors, strbuf_t* errmsg) {
    if (errors == NULL) {
        return SQLITE_MISUSE;
    }

    int result;

    int exists = 0;
    result = sql_check_table_exists(db, db_name, table_info->name, &exists);
    if (result != SQLITE_OK) {
        return result;
    }

    if (exists) {
        result = sql_check_table_schema(db, db_name, table_info, errors, errmsg);
    } else {
        result = sql_create_table(db, db_name, table_info, errors, errmsg);
    }

    if (result == SQLITE_OK && *errors == 0) {
        result = sql_insert_data(db, db_name, table_info, errors, errmsg);
    }

    return result;
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
