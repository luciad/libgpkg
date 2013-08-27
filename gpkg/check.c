#include "check.h"
#include "tables.h"

static int integrity_check_row(sqlite3_stmt *stmt, void *data) {
    const unsigned char* row = sqlite3_column_text(stmt, 0);
    if ( sqlite3_strnicmp(row, "ok", 3) != 0 ) {
        error_append( (error_t*)data, "integrity: %s", row );
    }
}

static int integrity_check(sqlite3 *db, char *db_name, error_t *error) {
    return sql_exec_stmt(db, integrity_check_row, NULL, error, "PRAGMA integrity_check");
}

static int table_definitions(sqlite3 *db, char *db_name, error_t *error) {
    int result = SQLITE_OK;

    const table_info_t * const *table = tables;
    while (*table != NULL) {
        result = sql_check_table(db, db_name, *table, error);
        if (result != SQLITE_OK) {
            break;
        }
        table++;
    }

    return result;
}

typedef int(*check_func)(sqlite3 *db, char* db_name, error_t *error);

static check_func checks[] = {
  integrity_check,
  table_definitions,
  NULL
};

int check_gpkg(sqlite3 *db, char *db_name, error_t *error) {
    int result = SQLITE_OK;

    check_func *current_func = checks;
    while( *current_func != NULL ) {
      result = (*current_func)(db, db_name, error);
      if (result != SQLITE_OK) {
          break;
      }
      current_func++;
    }

    return result;
}