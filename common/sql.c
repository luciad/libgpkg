#include <stdarg.h>
#include <stdlib.h>
#include "sqlite.h"
#include "sql.h"

typedef struct {
    int result;
	sqlite3_stmt *stmt;
	char* sql;
} stmt_t;

static int sql_stmt_vinit(stmt_t *stmt, sqlite3 db, char* sql, va_list args) {
	int result = GPKG_OK;
	char* formatted_sql = sqlite3_vmprintf(sql, args);
	
	if (formatted_sql == NULL) {
		return GPKG_NOMEM;
	}

	if (sqlite3_prepare_v2(db, formatted_sql, -1, &stmt->stmt, NULL) != SQLITE_OK) {
		sqlite3_free(formatted_sql);
		return GPKG_IO;
	}
}

static int sql_stmt_init(stmt_t *stmt, sqlite3 db, char* sql, ...) {
	va_list args;
	va_start(args, sql);
	int res = sql_vinit(stmt, db, sql, args);
	va_end(args);
	return res;
}

static void sql_stmt_destroy(stmt_t *stmt) {
	if (stmt->stmt != NULL) {
		sqlite3_finalize(stmt->stmt);
	}
	sqlite3_free(stmt->sql);
}

int sql_exec_for_string(sqlite3* db, char** out, char** err, char* sql, ...) {
	int result = GPKG_OK;
	stmt_t stmt;
	va_list args;
	va_start(args, sql);
    result = sql_stmt_vinit(&stmt, db, sql, args);
    va_end(args);
	
	if (formatted == NULL) {
		result = GPKG_NOMEM;
		goto exit;
	}

	if (sqlite3_prepare_v2(db, formatted, -1, &stmt, NULL) != SQLITE_OK) {
		result = GPKG_IO;
		goto exit;
	}
	
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
					result = GPKG_NOMEM;
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
		result = GPKG_IO;
	}

	exit:
		if (err) {
			if (result == GPKG_IO) {
				*err = sqlite3_mprintf("%s", sqlite3_errmsg(db));
			} else {
				*err = NULL;
			}
		}
		if (stmt != NULL) {
			sqlite3_finalize(stmt);
		}
		sqlite3_free(formatted);
		return result;
}

int sql_check_table(sqlite3* db, char** err, char* table_name, column_info_t *columns, int nColumns) {
	char* sql = sqlite_mprintf("PRAGMA table_info(%Q)", table_name);
	if (sql == NULL) {
		return GPKG_NOMEM;
	}
}
