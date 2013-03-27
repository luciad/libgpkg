#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite.h"
#include "sql.h"

typedef struct {
    int result;
    sqlite3_stmt *stmt;
    char *sql;
} stmt_t;

static int sql_stmt_vinit(stmt_t *stmt, sqlite3 *db, char *sql, va_list args) {
    char *formatted_sql = sqlite3_vmprintf(sql, args);

    if (formatted_sql == NULL) {
        return SQLITE_NOMEM;
    }

    if (sqlite3_prepare_v2(db, formatted_sql, -1, &stmt->stmt, NULL) != SQLITE_OK) {
        sqlite3_free(formatted_sql);
        return SQLITE_IOERR;
    }

    stmt->sql = formatted_sql;
    return SQLITE_OK;
}

static int sql_stmt_init(stmt_t *stmt, sqlite3 *db, char *sql, ...) {
    va_list args;
    va_start(args, sql);
    int res = sql_stmt_vinit(stmt, db, sql, args);
    va_end(args);
    return res;
}

static void sql_stmt_destroy(stmt_t *stmt) {
    if (stmt->stmt != NULL) {
        sqlite3_finalize(stmt->stmt);
        stmt->stmt = NULL;
    }
    sqlite3_free(stmt->sql);
}

int sql_exec_for_string(sqlite3 *db, char **out, char **err, char *sql, ...) {
    int result;
    stmt_t stmt;

    va_list args;
    va_start(args, sql);
    result = sql_stmt_vinit(&stmt, db, sql, args);
    va_end(args);

    if (result != SQLITE_OK) {
        goto exit;
    }

    int stmt_res = sqlite3_step(stmt.stmt);
    if (stmt_res == SQLITE_ROW) {
        int col_count = sqlite3_column_count(stmt.stmt);
        if (col_count > 0) {
            int length = sqlite3_column_bytes(stmt.stmt, 0);
            if (length <= 0) {
                *out = NULL;
            } else {
                const unsigned char *text = sqlite3_column_text(stmt.stmt, 0);
                *out = sqlite3_malloc(length + 1);
                if (*out == NULL) {
                    result = SQLITE_NOMEM;
                    goto exit;
                }
                memmove(*out, text, length + 1);
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
    if (err) {
        if (result != SQLITE_OK) {
            *err = sqlite3_mprintf("%s", sqlite3_errmsg(db));
        } else {
            *err = NULL;
        }
    }
    sql_stmt_destroy(&stmt);
    return result;
}

static int sql_check_cols(sqlite3_stmt *stmt, strbuf_t *errors, table_info_t *table_info) {
    int found[table_info->nColumns];
    memset(found, 0, table_info->nColumns * sizeof(int));
    int result;

    result = sqlite3_step(stmt);
    if (result == SQLITE_DONE) {
        strbuf_append(errors, "Table %Q does not exist\n", table_info->name);
        return SQLITE_OK;
    }

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
            if (strcasecmp(table_info->columns[c].name, name) == 0) {
                index = c;
                break;
            }
        }

        if (index != -1) {
            char *type = (char*)sqlite3_column_text(stmt, 2);
            if (strcasecmp(table_info->columns[index].type, type) != 0) {
                strbuf_append(errors, "Column %Q.%Q has incorrect type (expected: %s, actual: %s)\n", table_info->name, name, table_info->columns[index].type, type);
            }

            int not_null = sqlite3_column_int(stmt, 3);
            if (not_null != 0 && (table_info->columns[index].flags & SQL_NOT_NULL) == 0) {
                strbuf_append(errors, "Column %Q.%Q should not have 'not null' constraint\n", table_info->name, name);
            } else if (not_null == 0 && table_info->columns[index].flags & SQL_NOT_NULL) {
                strbuf_append(errors, "Column %Q.%Q should have 'not null' constraint\n", table_info->name, name);
            }

            int pk = sqlite3_column_int(stmt, 5);
            if (pk != 0 && (table_info->columns[index].flags & SQL_PRIMARY_KEY) == 0) {
                strbuf_append(errors, "Column %Q.%Q should not be part of primary key\n", table_info->name, name);
            } else if (pk == 0 && table_info->columns[index].flags & SQL_PRIMARY_KEY) {
                strbuf_append(errors, "Column %Q.%Q should be part of primary key\n", table_info->name, name);
            }
            found[index] = 1;
        } else {
            strbuf_append(errors, "Redundant column %Q.%Q\n", table_info->name, name);
        }

        result = sqlite3_step(stmt);
    }

    if (result != SQLITE_DONE) {
        return result;
    }

    for (int i = 0; i < table_info->nColumns; i++) {
        if (found[i] == 0) {
            strbuf_append(errors, "Column %Q.%Q is missing\n", table_info->name, table_info->columns[i].name);
        }
    }
    return SQLITE_OK;
}

int sql_check_table(sqlite3 *db, strbuf_t *errors, table_info_t* table_info) {
    stmt_t stmt;
    int result;

    result = sql_stmt_init(&stmt, db, "PRAGMA table_info(%Q)", table_info->name);
    if (result != SQLITE_OK) {
        goto exit;
    }

    result = strbuf_init(errors, 128);
    if (result != SQLITE_OK) {
        goto exit;
    }

    result = sql_check_cols(stmt.stmt, errors, table_info);

    exit:
    sql_stmt_destroy(&stmt);
    return result;
}

int sql_create_table(sqlite3 *db, strbuf_t *errors, table_info_t* table_info) {
    int result;

    strbuf_t sql;
    result = strbuf_init(&sql, 4096);
    if (result != SQLITE_OK) {
        return result;
    }

    int has_pk = 0;

    strbuf_append(&sql, "CREATE TABLE IF NOT EXISTS %Q (", table_info->name);
    column_info_t *columns = table_info->columns;
    for(int i = 0; i < table_info->nColumns; i++) {
        if (i > 0) {
            strbuf_append(&sql, ",\n  %Q %s", columns[i].name, columns[i].type);
        } else {
            strbuf_append(&sql, "\n  %Q %s", columns[i].name, columns[i].type);
        }
        if (columns[i].flags & SQL_NOT_NULL) {
            strbuf_append(&sql, " NOT NULL");
        }
        switch (columns[i].default_value_type) {
            default:
                break;
            case SQLITE_TEXT:
                strbuf_append(&sql, " DEFAULT %Q", columns[i].default_value.text_value);
                break;
            case SQLITE_FLOAT:
                strbuf_append(&sql, " DEFAULT %g", columns[i].default_value.double_value);
                break;
            case SQLITE_INTEGER:
                strbuf_append(&sql, " DEFAULT %d", columns[i].default_value.int_value);
                break;
        }

        if (columns[i].column_constraints != NULL) {
            strbuf_append(&sql, " %s", columns[i].column_constraints);
        }

        if (columns[i].flags & SQL_PRIMARY_KEY) {
            has_pk = 1;
        }
    }

    if (has_pk) {
        strbuf_append(&sql, ",\n  PRIMARY KEY (");
        int first = 1;
        for(int i = 0; i < table_info->nColumns; i++) {
            if (columns[i].flags & SQL_PRIMARY_KEY) {
                if (first) {
                    first = 0;
                    strbuf_append(&sql, "%Q", columns[i].name);
                } else {
                    strbuf_append(&sql, ", %Q", columns[i].name);
                }
            }
        }
        strbuf_append(&sql, ")");
    }

    strbuf_append(&sql, "\n)");

    char* err = NULL;
    result = sqlite3_exec(db, strbuf_data(&sql), NULL, NULL, &err);
    if (err) {
        strbuf_append(errors, err);
    }

    return result;
}
