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
#include <stdio.h>
#include "check.h"
#include "sql.h"

static int integrity_check_row(sqlite3 *db, sqlite3_stmt *stmt, void *data) {
  const char *row = (const char *)sqlite3_column_text(stmt, 0);
  if (sqlite3_strnicmp(row, "ok", 3) != 0) {
    error_append((error_t *)data, "integrity: %s", row);
  }
  return SQLITE_OK;
}

static int integrity_check(sqlite3 *db, const char *db_name, error_t *error) {
  return sql_exec_stmt(db, integrity_check_row, NULL, error, "PRAGMA integrity_check");
}

typedef struct {
  const char *db_name;
  error_t *error;
} foreign_key_check_data;

static int foreign_key_check_row(sqlite3 *db, sqlite3_stmt *stmt, void *data) {
  int result = SQLITE_OK;
  foreign_key_check_data *d = (foreign_key_check_data *)data;
  foreign_key_info_t info;
  char *value = NULL;

  sql_foreign_key_info_init(&info);

  char *table = sqlite3_mprintf("%s", sqlite3_column_text(stmt, 0));
  sqlite3_int64 rowId = sqlite3_column_int64(stmt, 1);
  char *referred_table = sqlite3_mprintf("%s", sqlite3_column_text(stmt, 2));
  int foreign_key_index = sqlite3_column_int(stmt, 3);

  result = sql_foreign_key_info(db, d->db_name, table, foreign_key_index, &info, d->error);
  if (result != SQLITE_OK) {
    goto exit;
  }

  result = sql_exec_for_string(db, &value, "SELECT \"%w\" FROM \"%w\".\"%w\" WHERE ROWID = %d", info.from_column, d->db_name, table, rowId);
  if (result != SQLITE_OK) {
    goto exit;
  }

  error_append(d->error, "%s: foreign key from '%s' to '%s.%s' failed for value '%s'", table, info.from_column, referred_table, info.to_column, value);

exit:
  sql_foreign_key_info_destroy(&info);
  sqlite3_free(table);
  sqlite3_free(referred_table);
  sqlite3_free(value);

  return result;
}

static int foreign_key_check(sqlite3 *db, const char *db_name, error_t *error) {
  foreign_key_check_data data = {
    db_name,
    error
  };
  return sql_exec_stmt(db, foreign_key_check_row, NULL, &data, "PRAGMA foreign_key_check");
}

typedef int(*check_func)(sqlite3 *db, const char *db_name, error_t *error);

static check_func checks[] = {
  integrity_check,
  foreign_key_check,
  NULL
};

int check_integrity(sqlite3 *db, const char *db_name, error_t *error) {
  int result = SQLITE_OK;

  check_func *current_func = checks;
  while (*current_func != NULL) {
    result = (*current_func)(db, db_name, error);
    if (result != SQLITE_OK) {
      break;
    }
    current_func++;
  }

  return result;
}

int check_database(sqlite3 *db, const char *db_name, const table_info_t *const *tables, error_t *error) {
  int result = SQLITE_OK;

  const table_info_t *const *table = tables;
  while (*table != NULL) {
    result = sql_check_table(db, db_name, *table, error);
    if (result != SQLITE_OK) {
      break;
    }
    table++;
  }

  return result;
}

int init_database(sqlite3 *db, const char *db_name, const table_info_t *const *tables, error_t *error) {
  int result = SQLITE_OK;
  const table_info_t *const *table = tables;

  while (*table != NULL) {
    result = sql_init_table(db, db_name, *table, error);
    if (result != SQLITE_OK) {
      break;
    }
    table++;
  }

  if (result == SQLITE_OK && error_count(error) > 0) {
    return SQLITE_ERROR;
  } else {
    return result;
  }
}