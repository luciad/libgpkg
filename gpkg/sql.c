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

static int sql_stmt_exec(sqlite3 *db, sql_callback row, sql_callback nodata, void *data, char *sql, va_list args) {
  sqlite3_stmt *stmt = NULL;
  int result = sql_stmt_vinit(&stmt, db, sql, args);

  if (result != SQLITE_OK) {
    return result;
  }

  int stmt_res = sqlite3_step(stmt);
  if (stmt_res == SQLITE_DONE) {
    if (nodata != NULL) {
      int callback_res = nodata(db, stmt, data);
      if (callback_res != SQLITE_ABORT) {
        stmt_res = callback_res;
      }
    }
  } else {
    if (row == NULL) {
      while (stmt_res == SQLITE_ROW) {
        stmt_res = sqlite3_step(stmt);
      }
    } else {
      while (stmt_res == SQLITE_ROW) {
        int callback_res = row(db, stmt, data);
        if (callback_res == SQLITE_ABORT) {
          stmt_res = SQLITE_DONE;
        } else if (callback_res != SQLITE_OK) {
          stmt_res = callback_res;
        } else {
          stmt_res = sqlite3_step(stmt);
        }
      }
    }
  }

  if (stmt_res != SQLITE_DONE) {
    result = stmt_res;
  }

  sql_stmt_destroy(stmt);
  return result;
}

static int row_string(sqlite3 *db, sqlite3_stmt *stmt, void *data) {
  char **out = (char **)data;
  int col_count = sqlite3_column_count(stmt);
  if (col_count > 0) {
    const unsigned char *text = sqlite3_column_text(stmt, 0);
    int length = sqlite3_column_bytes(stmt, 0);
    if (length <= 0) {
      *out = NULL;
    } else {
      *out = (char *)sqlite3_malloc(length + 1);
      if (*out == NULL) {
        return SQLITE_NOMEM;
      }
      memmove(*out, text, (size_t)length + 1);
    }
    return SQLITE_ABORT;
  } else {
    return SQLITE_MISUSE;
  }
}

static int nodata_string(sqlite3 *db, sqlite3_stmt *stmt, void *out) {
  *((char **)out) = NULL;
  return SQLITE_ABORT;
}

int sql_exec_for_string(sqlite3 *db, char **out, char *sql, ...) {
  va_list args;
  va_start(args, sql);
  int result = sql_stmt_exec(db, row_string, nodata_string, out, sql, args);
  va_end(args);
  return result;
}

static int row_int(sqlite3 *db, sqlite3_stmt *stmt, void *data) {
  int col_count = sqlite3_column_count(stmt);
  if (col_count > 0) {
    *((int *)data) = sqlite3_column_int(stmt, 0);
    return SQLITE_ABORT;
  } else {
    return SQLITE_MISUSE;
  }
}

static int nodata_int(sqlite3 *db, sqlite3_stmt *stmt, void *data) {
  *((int *)data) = 0;
  return SQLITE_ABORT;
}

int sql_exec_for_int(sqlite3 *db, int *out, char *sql, ...) {
  va_list args;
  va_start(args, sql);
  int result = sql_stmt_exec(db, row_int, nodata_int, out, sql, args);
  va_end(args);
  return result;
}

static int row_double(sqlite3 *db, sqlite3_stmt *stmt, void *data) {
  int col_count = sqlite3_column_count(stmt);
  if (col_count > 0) {
    *((double *)data) = sqlite3_column_double(stmt, 0);
    return SQLITE_ABORT;
  } else {
    return SQLITE_MISUSE;
  }
}

static int nodata_double(sqlite3 *db, sqlite3_stmt *stmt, void *data) {
  *((double *)data) = 0.0;
  return SQLITE_ABORT;
}

int sql_exec_for_double(sqlite3 *db, double *out, char *sql, ...) {
  va_list args;
  va_start(args, sql);
  int result = sql_stmt_exec(db, row_double, nodata_double, out, sql, args);
  va_end(args);
  return result;
}

static int abort_after_first_row(sqlite3 *db, sqlite3_stmt *stmt, void *data) {
  return SQLITE_ABORT;
}

int sql_exec(sqlite3 *db, char *sql, ...) {
  va_list args;
  va_start(args, sql);
  int result = sql_stmt_exec(db, abort_after_first_row, NULL, NULL, sql, args);
  va_end(args);
  return result;
}

int sql_exec_all(sqlite3 *db, char *sql, ...) {
  va_list args;
  va_start(args, sql);
  int result = sql_stmt_exec(db, NULL, NULL, NULL, sql, args);
  va_end(args);
  return result;
}

int sql_exec_stmt(sqlite3 *db, sql_callback row, sql_callback nodata, void *data, char *sql, ...) {
  va_list args;
  va_start(args, sql);
  int result = sql_stmt_exec(db, row, nodata, data, sql, args);
  va_end(args);
  return result;
}

static int sql_check_table_exists_nodata(sqlite3 *db, sqlite3_stmt *stmt, void *data) {
  *((int *) data) = 0;
  return SQLITE_ABORT;
}

static int sql_check_table_exists_row(sqlite3 *db, sqlite3_stmt *stmt, void *data) {
  *((int *) data) = 1;
  return SQLITE_ABORT;
}

int sql_check_table_exists(sqlite3 *db, const char *db_name, const char *table_name, int *exists) {
  int result = sql_exec_stmt(db, sql_check_table_exists_row, sql_check_table_exists_nodata, exists, "PRAGMA \"%w\".table_info(\"%w\")", db_name, table_name);
  if (result != SQLITE_OK) {
    *exists = 0;
  }

  return result;
}

typedef struct {
  int found;
  const char *name;
} column_to_find_t;

static int sql_check_column_exists_row(sqlite3 *db, sqlite3_stmt *stmt, void *data) {
  column_to_find_t *c = (column_to_find_t *)data;
  if (sqlite3_strnicmp(c->name, (const char *)sqlite3_column_text(stmt, 1), strlen(c->name) + 1) == 0) {
    c->found = 1;
  }
  return SQLITE_OK;
}

int sql_check_column_exists(sqlite3 *db, const char *db_name, const char *table_name, const char *column_name, int *exists) {
  column_to_find_t c;
  c.found = 0;
  c.name = column_name;
  if (c.name == NULL) {
    return SQLITE_ERROR;
  }

  int result = sql_exec_stmt(
                 db, sql_check_column_exists_row, NULL, &c,
                 "PRAGMA \"%w\".table_info(\"%w\")", db_name, table_name
               );

  *exists = c.found;

  return result;
}

static int sql_count_columns(const table_info_t *table_info) {
  int nColumns = 0;
  const column_info_t *column = table_info->columns;
  while (column->name != NULL) {
    column++;
    nColumns++;
  }
  return nColumns;
}

typedef struct {
  errorstream_t *error;
  int *cols_found;
  int nColumns;
  const table_info_t *table_info;
  int flags;
} check_cols_data;

static int sql_check_cols_row(sqlite3 *db, sqlite3_stmt *stmt, void *data) {
  check_cols_data *check = (check_cols_data *)data;
  errorstream_t *error = check->error;
  int *found = check->cols_found;
  int nColumns = check->nColumns;
  const table_info_t *table_info = check->table_info;

  // 0 index
  // 1 name
  // 2 type
  // 3 not null
  // 4 default value
  // 5 primary key
  char *name = (char *)sqlite3_column_text(stmt, 1);
  int index = -1;
  for (int c = 0; c < nColumns; c++) {
    if (sqlite3_strnicmp(table_info->columns[c].name, name, strlen(table_info->columns[c].name) + 1) == 0) {
      index = c;
      break;
    }
  }

  if (index != -1) {
    char *type = (char *)sqlite3_column_text(stmt, 2);
    if (sqlite3_strnicmp(table_info->columns[index].type, type, strlen(table_info->columns[index].type) + 1) != 0) {
      error_append(error, "Column %s.%s has incorrect type (expected: %s, actual: %s)", table_info->name, name, table_info->columns[index].type, type);
    }

    int not_null = sqlite3_column_int(stmt, 3);
    if (not_null != 0 && (table_info->columns[index].flags & SQL_NOT_NULL) == 0) {
      error_append(error, "Column %s.%s should not have 'not null' constraint\n", table_info->name, name);
    } else if (not_null == 0 && table_info->columns[index].flags & SQL_NOT_NULL) {
      error_append(error, "Column %s.%s should have 'not null' constraint", table_info->name, name);
    }

    if ((check->flags & SQL_CHECK_DEFAULT_VALUES) != 0) {
      value_t default_value = table_info->columns[index].default_value;
      if (default_value.type == VALUE_TEXT) {
        char *expected = sqlite3_mprintf("'%s'", VALUE_AS_TEXT(default_value));
        if (sqlite3_column_type(stmt, 4) == SQLITE_NULL) {
          error_append(error, "Column %s.%s has incorrect default value: expected '%s' but was NULL", table_info->name, name, expected);
        } else {
          const char *actual = (const char *)sqlite3_column_text(stmt, 4);
          if (sqlite3_strnicmp(expected, actual, strlen(expected) + 1) != 0) {
            error_append(error, "Column %s.%s has incorrect default value: expected '%s' but was '%s'", table_info->name, name, expected, actual);
          }
        }
        sqlite3_free(expected);
      } else if (default_value.type == VALUE_FUNC) {
        char *expected = sqlite3_mprintf(VALUE_AS_TEXT(default_value));
        if (sqlite3_column_type(stmt, 4) == SQLITE_NULL) {
          error_append(error, "Column %s.%s has incorrect default value: expected '%s' but was NULL", table_info->name, name, expected);
        } else {
          const char *actual = (const char *)sqlite3_column_text(stmt, 4);
          if (sqlite3_strnicmp(expected, actual, strlen(expected) + 1) != 0) {
            error_append(error, "Column %s.%s has incorrect default value: expected '%s' but was '%s'", table_info->name, name, expected, actual);
          }
        }
        sqlite3_free(expected);
      } else if (default_value.type == VALUE_INTEGER) {
        int expected = VALUE_AS_INT(default_value);
        if (sqlite3_column_type(stmt, 4) == SQLITE_NULL) {
          error_append(error, "Column %s.%s has incorrect default value: expected %d but was NULL", table_info->name, name, expected);
        } else {
          int actual = sqlite3_column_int(stmt, 4);
          if (actual != expected) {
            error_append(error, "Column %s.%s has incorrect default value: expected %d but was %d", table_info->name, name, expected, actual);
          }
        }
      } else if (default_value.type == VALUE_DOUBLE) {
        double expected = VALUE_AS_DOUBLE(default_value);
        if (sqlite3_column_type(stmt, 4) == SQLITE_NULL) {
          error_append(error, "Column %s.%s has incorrect default value: expected %f but was NULL", table_info->name, name, expected);
        } else {
          double actual = sqlite3_column_double(stmt, 4);
          if (actual != expected) {
            error_append(error, "Column %s.%s has incorrect default value: expected %f but was %f", table_info->name, name, expected, actual);
          }
        }
      } else if (default_value.type == VALUE_NULL) {
        if (sqlite3_column_type(stmt, 4) != SQLITE_NULL) {
          const char *actual = (const char *)sqlite3_column_text(stmt, 4);
          error_append(error, "Column %s.%s has incorrect default value: expected NULL but was %s", table_info->name, name, actual);
        }
      }
    }

    int pk = sqlite3_column_int(stmt, 5);
    if (pk != 0 && (table_info->columns[index].flags & SQL_PRIMARY_KEY) == 0) {
      error_append(error, "Column %s.%s should not be part of primary key", table_info->name, name);
    } else if (pk == 0 && table_info->columns[index].flags & SQL_PRIMARY_KEY) {
      error_append(error, "Column %s.%s should be part of primary key", table_info->name, name);
    }
    found[index] = 1;
  } else {
    error_append(error, "Redundant column %s.%s", table_info->name, name);
  }

  return SQLITE_OK;
}

static int sql_check_table_schema(sqlite3 *db, const char *db_name, const table_info_t *table_info, int check_flags, errorstream_t *error) {
  int nColumns = sql_count_columns(table_info);
  int *found = (int *)sqlite3_malloc(nColumns * sizeof(int));
  if (found == NULL) {
    return SQLITE_NOMEM;
  }
  memset(found, 0, nColumns * sizeof(int));
  check_cols_data data = { error, found, nColumns, table_info, check_flags };

  int result = sql_exec_stmt(db, sql_check_cols_row, NULL, &data, "PRAGMA \"%w\".table_info(\"%w\")", db_name, table_info->name);

  if (result == SQLITE_OK) {
    for (int i = 0; i < nColumns; i++) {
      if (found[i] == 0) {
        if (error) {
          error_append(error, "Column %s.%s is missing\n", table_info->name, table_info->columns[i].name);
        }
      }
    }
  }

  sqlite3_free(found);

  return result;
}

static int sql_format_missing_row(const char *db_name, const table_info_t *table_info, const value_t *row, errorstream_t *error) {
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

static int sql_check_data(sqlite3 *db, const char *db_name, const table_info_t *table_info, errorstream_t *error) {
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
  for (int i = 0; i < nColumns; i++) {
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

  for (size_t rIx = 0; rIx < table_info->nRows; rIx++) {
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

static int sql_format_insert_data(const char *db_name, const table_info_t *table_info, char **query) {
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
  for (int i = 0; i < nColumns; i++) {
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

  for (int i = 0; i < nColumns; i++) {
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

static int sql_insert_data(sqlite3 *db, const char *db_name, const table_info_t *table_info, errorstream_t *error) {
  if (table_info->nRows <= 0) {
    return SQLITE_OK;
  }

  sqlite3_stmt *stmt = NULL;
  int result = SQLITE_OK;
  char *query = NULL;

  result = sql_format_insert_data(db_name, table_info, &query);
  if (result != SQLITE_OK) {
    goto exit;
  }

  result = sql_stmt_init(&stmt, db, query);
  if (result != SQLITE_OK) {
    goto exit;
  }

  int nColumns = sql_count_columns(table_info);

  for (size_t rIx = 0; rIx < table_info->nRows; rIx++) {
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
#define SQL_IS_CONSTRAINT(flags, mask, ix) ((flags & mask) && (ix == -1 || SQL_CONSTRAINT_IX(flags) == ix))

static void appendTableConstraint(const table_info_t *table_info, strbuf_t *sql, int constraint_mask, int constraint_idx) {
  char *constraint_name;
  if (constraint_mask == SQL_PRIMARY_KEY_MASK) {
    constraint_name = "PRIMARY KEY";
  } else if (constraint_mask == SQL_UNIQUE_MASK) {
    constraint_name = "UNIQUE";
  } else {
    return;
  }

  int has_cols = 0;
  int nColumns = sql_count_columns(table_info);

  for (int i = 0; i < nColumns; i++) {
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
  for (int i = 0; i < nColumns; i++) {
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

static int sql_create_table(sqlite3 *db, const char *db_name, const table_info_t *table_info, errorstream_t *error) {
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
  for (int i = 0; i < nColumns; i++) {
    if (columns[i].flags & SQL_PRIMARY_KEY_MASK) {
      pk_count++;
    }
  }

  strbuf_append(&sql, "CREATE TABLE IF NOT EXISTS \"%w\".\"%w\" (", db_name, table_info->name);
  for (int i = 0; i < nColumns; i++) {
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
    appendTableConstraint(table_info, &sql, SQL_PRIMARY_KEY_MASK, -1);
  }

  if (max_uk > 0) {
    for (int i = 0; i <= max_uk; i++) {
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

#define SQL_CREATE 1

static int sql_init_check_table(sqlite3 *db, const char *db_name, const table_info_t *table_info, int flags, errorstream_t *error) {
  if (error == NULL) {
    return SQLITE_MISUSE;
  }

  int exists = 0;
  int result = sql_check_table_exists(db, db_name, table_info->name, &exists);
  if (result == SQLITE_OK) {
    if (exists) {
      result = sql_check_table_schema(db, db_name, table_info, flags, error);

      if (result == SQLITE_OK && (flags & SQL_CREATE) != 0) {
        result = sql_insert_data(db, db_name, table_info, error);
      }

      if (result == SQLITE_OK && (flags & SQL_CHECK_DEFAULT_DATA) != 0) {
        result = sql_check_data(db, db_name, table_info, error);
      }
    } else if ((flags & SQL_MUST_EXIST) != 0) {
      if ((flags & SQL_CREATE) == 0) {
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

int sql_check_table(sqlite3 *db, const char *db_name, const table_info_t *table_info, int check_flags, errorstream_t *error) {
  return sql_init_check_table(db, db_name, table_info, check_flags & ~SQL_CREATE, error);
}

int sql_init_table(sqlite3 *db, const char *db_name, const table_info_t *table_info, errorstream_t *error) {
  return sql_init_check_table(db, db_name, table_info, SQL_CREATE | SQL_MUST_EXIST, error);
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

int sql_init_stmt(sqlite3_stmt **stmt, sqlite3 *db, char *sql) {
  return sql_stmt_init(stmt, db, sql);
}

static int sql_integrity_check_row(sqlite3 *db, sqlite3_stmt *stmt, void *data) {
  const char *row = (const char *)sqlite3_column_text(stmt, 0);
  if (sqlite3_strnicmp(row, "ok", 3) != 0) {
    error_append((errorstream_t *)data, "integrity: %s", row);
  }
  return SQLITE_OK;
}

static int sql_integrity_check(sqlite3 *db, const char *db_name, errorstream_t *error) {
  return sql_exec_stmt(db, sql_integrity_check_row, NULL, error, "PRAGMA integrity_check");
}

typedef struct {
  int id;
  int seq;
  const char *table;
  const char *from_column;
  const char *to_column;
} foreign_key_info_t;

typedef struct {
  foreign_key_info_t *info;
  int index;
  int found;
} sql_foreign_key_info_data;

static int sql_foreign_key_info_row(sqlite3 *db, sqlite3_stmt *stmt, void *data) {
  sql_foreign_key_info_data *d = (sql_foreign_key_info_data *)data;
  int index = sqlite3_column_int(stmt, 0);
  if (index == d->index) {
    d->found = 1;
    d->info->id = index;
    d->info->seq = sqlite3_column_int(stmt, 1);
    d->info->table = sqlite3_mprintf("%s", sqlite3_column_text(stmt, 2));
    d->info->from_column = sqlite3_mprintf("%s", sqlite3_column_text(stmt, 3));
    d->info->to_column = sqlite3_mprintf("%s", sqlite3_column_text(stmt, 4));
    return SQLITE_ABORT;
  } else {
    return SQLITE_OK;
  }
}

static int sql_foreign_key_info(sqlite3 *db, const char *db_name, const char *table_name, int index, foreign_key_info_t *info, errorstream_t *error) {
  sql_foreign_key_info_data data;
  memset(&data, 0, sizeof(sql_foreign_key_info_data));
  data.info = info;
  data.index = index;
  data.found = 0;

  int result = sql_exec_stmt(
                 db, sql_foreign_key_info_row, NULL, &data,
                 "PRAGMA \"%w\".foreign_key_list(\"%w\")", db_name, table_name
               );
  if (result == SQLITE_OK && !data.found) {
    error_append(error, "Could not find foreign key in table %s with index %d", table_name, index);
    result = SQLITE_ERROR;
  }

  return result;
}

typedef struct {
  const char *db_name;
  errorstream_t *error;
} foreign_key_check_data;

static int sql_foreign_key_check_row(sqlite3 *db, sqlite3_stmt *stmt, void *data) {
  int result = SQLITE_OK;
  foreign_key_check_data *d = (foreign_key_check_data *)data;
  foreign_key_info_t info;
  char *value = NULL;

  memset(&info, 0, sizeof(foreign_key_info_t));

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
  sqlite3_free((void *)info.table);
  sqlite3_free((void *)info.from_column);
  sqlite3_free((void *)info.to_column);
  sqlite3_free(table);
  sqlite3_free(referred_table);
  sqlite3_free(value);

  return result;
}

static int sql_foreign_key_check(sqlite3 *db, const char *db_name, errorstream_t *error) {
  foreign_key_check_data data = {
    db_name,
    error
  };
  return sql_exec_stmt(db, sql_foreign_key_check_row, NULL, &data, "PRAGMA foreign_key_check");
}

typedef int(*check_func)(sqlite3 *db, const char *db_name, errorstream_t *error);

static check_func checks[] = {
  sql_integrity_check,
  sql_foreign_key_check,
  NULL
};

int sql_check_integrity(sqlite3 *db, const char *db_name, errorstream_t *error) {
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

int sql_create_function(sqlite3 *db, const char *name, void (*function)(sqlite3_context *, int, sqlite3_value **), int args, int flags, void *user_data, void (*destroy)(void *), errorstream_t *error) {
  int function_flags = SQLITE_UTF8;

#if SQLITE_VERSION_NUMBER >= 3008003
  if (((flags & SQL_DETERMINISTIC) != 0) && sqlite3_libversion_number() >= 3008003) {
    function_flags |= SQLITE_DETERMINISTIC;
  }
#endif

  int result = sqlite3_create_function_v2(
                 db, name, args, function_flags, user_data, function, NULL, NULL, destroy
               );
  if (result != SQLITE_OK) {
    error_append(error, "Error registering function %s/%d: %s", name, args, sqlite3_errmsg(db));
  }

  return result;
}
