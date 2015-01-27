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
#include "error.h"
#include "strbuf.h"

/**
 * \addtogroup sql SQL utilities
 * @{
 */

/**
 * Column flag that indicates the column may not contain NULL values.
 */
#define SQL_NOT_NULL 0x1
/**
 * Column flag that indicates the column is part of the primary key of the table.
 */
#define SQL_PRIMARY_KEY 0x2

/**
 * Column flag that indicates the column is part of the primary key of the table.
 */
#define SQL_AUTOINCREMENT 0x4

/**
 * Macro to generate a column flag that indicates the column is part of a unique constraint. The index value
 * indicates which unique constraint the column is part of. Index must be greater than 0.
 */
#define SQL_UNIQUE(i) (0x8 | ((i & 0xF) << 4))

/**
 * Enumeration of value types.
 */
typedef enum {
  /**
   * Indicates the value of a value_t struct contains a text value.
   */
  VALUE_TEXT,
  /**
   * Indicates the value of a value_t struct contains a function name value.
   */
  VALUE_FUNC,
  /**
   * Indicates the value of a value_t struct contains an integer value.
   */
  VALUE_INTEGER,
  /**
   * Indicates the value of a value_t struct contains a double value.
   */
  VALUE_DOUBLE,
  /**
   * Indicates the value of a value_t struct contains a NULL value.
   */
  VALUE_NULL
} value_type_t;

/**
 * A SQL value.
 */
typedef struct {
  /**
   * The value as a string. This field is valid if type is VALUE_TEXT or VALUE_FUNC.
   */
  char *text;
  /**
   * The value as a double. This field is valid if type is VALUE_DOUBLE.
   */
  double dbl;
  /**
   * The value as an integer. This field is valid if type is VALUE_INTEGER.
   */
  int integer;

  /**
   * The type of this value.
   */
  value_type_t type;
} value_t;

/**
 * Macro to initialize a value_t containing a NULL value.
 */
#define NULL_VALUE {NULL, 0.0, 0, VALUE_NULL}

/**
 * Macro to initialize a value_t containing a string value.
 */
#define TEXT_VALUE(t) {t, 0.0, 0, VALUE_TEXT}
/**
 * Macro to initialize a value_t containing a function name value.
 */
#define FUNC_VALUE(t) {t, 0.0, 0, VALUE_FUNC}

/**
 * Macro to initialize a value_t containing a double value.
 */
#define DOUBLE_VALUE(t) {NULL, t, 0, VALUE_DOUBLE}

/**
 * Macro to initialize a value_t containing an integer value.
 */
#define INT_VALUE(t) {NULL, 0.0, t, VALUE_INTEGER}

/**
 * Macro to retrieve the NULL value contained in a value_t.
 */
#define VALUE_AS_NULL(v) (NULL)
/**
 * Macro to retrieve the string value contained in a value_t.
 */
#define VALUE_AS_TEXT(v) (v.text)
/**
 * Macro to retrieve the function name value contained in a value_t.
 */
#define VALUE_AS_FUNC(v) (v.text)
/**
 * Macro to retrieve the double value contained in a value_t.
 */
#define VALUE_AS_DOUBLE(v) (v.dbl)
/**
 * Macro to retrieve the integer value contained in a value_t.
 */
#define VALUE_AS_INT(v) (v.integer)

/**
 * Description of a column in a SQL table.
 */
typedef struct {
  /**
   * The name of the column.
   */
  const char *name;
  /**
   * The SQL type of the column.
   */
  const char *type;
  /**
   * The default value of the column. This field may be set to NULL.
   */
  const value_t default_value;
  /**
   * Bitwise OR of any flags that apply to this column. This field may contain a combination of:
   * \li SQL_NOT_NULL
   * \li SQL_PRIMARY_KEY
   * \li SQL_AUTOINCREMENT
   * \li SQL_UNIQUE
   */
  const int flags;
  /**
   * The column constraints that should be applied to the column as a SQL expression.
   */
  const char *column_constraints;
} column_info_t;

/**
 * Description of a SQL table.
 */
typedef struct {
  /**
   * The name of the table.
   */
  const char *name;
  /**
   * An array of column information. The array is terminated by a column_info_t instance that has all fields set
   * to NULL or zero.
   */
  const column_info_t *columns;
  /**
   * An array of rows that should be present in the table. Each row is an array of value_t and should contain
   * nColumns elements.
   */
  const value_t *rows;
  /**
   * The number of elements in rows.
   */
  const size_t nRows;
} table_info_t;

/**
 * Begins a named SQLite transaction. See the documentation on SQLite savepoints for more details.
 * @param db the SQLite database context
 * @param name the name of the transaction
 * @return SQLITE_OK if the transaction was started successfully\n
 *         A SQLite error code otherwise
 * @see sql_commit
 * @see sql_rollback
 */
int sql_begin(sqlite3 *db, char *name);

/**
 * Commits a named SQLite transaction. See the documentation on SQLite savepoints for more details.
 * @param db the SQLite database context
 * @param name the name of the transaction
 * @return SQLITE_OK if the transaction was successfully comitted\n
 *         A SQLite error code otherwise
 * @see sql_begin
 * @see sql_rollback
 */
int sql_commit(sqlite3 *db, char *name);

/**
 * Rolls back a named SQLite transaction. See the documentation on SQLite savepoints for more details.
 * @param db the SQLite database context
 * @param name the name of the transaction
 * @return SQLITE_OK if the transaction was successfully rolled back\n
 *         A SQLite error code otherwise
 * @see sql_begin
 * @see sql_commit
 */
int sql_rollback(sqlite3 *db, char *name);

/**
 * Executes a SQL statement. The SQL statement can be a printf style format pattern.
 * @param db the SQLite database context
 * @param sql the SQL statement to execute
 * @return SQLITE_OK if the SQL statement was executed successfully\n
 *         A SQLite error code otherwise
 */
int sql_exec(sqlite3 *db, char *sql, ...);

/**
 * Executes a SQL statement. The SQL statement can be a printf style format pattern.
 * @param db the SQLite database context
 * @param sql the SQL statement to execute
 * @return SQLITE_OK if the SQL statement was executed successfully\n
 *         A SQLite error code otherwise
 */
int sql_exec_all(sqlite3 *db, char *sql, ...);

/**
 * Callback function for sql_exec_stmt.
 * @param stmt the currently executing statement
 * @param data the user defined data that was passed to sql_exec_stmt
 * @return SQLITE_OK to continue iterating
 *         SQLITE_ABORT to abort iterating; sql_exec_stmt returns with SQLITE_OK
 *         A SQLite error code to abort iterating; sql_exec_stmt returns with the return value of this function
 */
typedef int(sql_callback)(sqlite3 *db, sqlite3_stmt *stmt, void *data);

/**
 * Executes a SQL statement. The SQL statement can be a printf style format pattern.
 * @param db the SQLite database context
 * @param row optional callback that is called for each row in the result set
 * @param nodata optional callback that is called when the result set is empty
 * @param data optional user data that is passed to the row and/or nodata callbacks
 * @param sql the SQL statement to execute
 * @return SQLITE_OK if the SQL statement was executed successfully\n
 *         A SQLite error code otherwise
 */
int sql_exec_stmt(sqlite3 *db, sql_callback *row, sql_callback *nodata, void *data, char *sql, ...);

/**
 * Executes a SQL statement that is expected to return a single string value. The SQL statement can be a printf style
 * format pattern.
 * @param db the SQLite database context
 * @param[out] out on success, out will be set to the returned string value
 * @param sql the SQL statement to execute
 * @return SQLITE_OK if the SQL statement was executed successfully\n
 *         A SQLite error code otherwise
 */
int sql_exec_for_string(sqlite3 *db, char **out, char *sql, ...);

/**
 * Executes a SQL statement that is expected to return a single integer value. The SQL statement can be a printf style
 * format pattern.
 * @param db the SQLite database context
 * @param[out] out on success, out will be set to the returned integer value
 * @param sql the SQL statement to execute
 * @return SQLITE_OK if the SQL statement was executed successfully\n
 *         A SQLite error code otherwise
 */
int sql_exec_for_int(sqlite3 *db, int *out, char *sql, ...);

/**
 * Executes a SQL statement that is expected to return a single double value. The SQL statement can be a printf style
 * format pattern.
 * @param db the SQLite database context
 * @param[out] out on success, out will be set to the returned double value
 * @param sql the SQL statement to execute
 * @return SQLITE_OK if the SQL statement was executed successfully\n
 *         A SQLite error code otherwise
 */
int sql_exec_for_double(sqlite3 *db, double *out, char *sql, ...);

/**
 * Checks if a table exists in the database.
 * @param db the SQLite database context
 * @param db_name the name of the attached database to use. This can be 'main', 'temp' or any attached database.
 * @param table_name the name of the table to check.
 * @param[out] exists on success, exists will be set to 1 if the table exists or to 0 otherwise
 * @return SQLITE_OK if the table check was successful\n
 *         A SQLite error code otherwise
 */
int sql_check_table_exists(sqlite3 *db, const char *db_name, const char *table_name, int *exists);

/**
 * Checks if a column exists in the given table of the database.
 * @param db the SQLite database context
 * @param db_name the name of the attached database to use. This can be 'main', 'temp' or any attached database.
 * @param table_name the name of the table to check.
 * @param column_name the name of the column to check.
 * @param[out] exists on success, exists will be set to 1 if the given table has a column with the given name or to 0 otherwise
 * @return SQLITE_OK if the column check was successful\n
 *         A SQLite error code otherwise
 */
int sql_check_column_exists(sqlite3 *db, const char *db_name, const char *table_name, const char *column_name, int *exists);

#define SQL_MUST_EXIST (1 << 1)
#define SQL_CHECK_DEFAULT_VALUES (1 << 2)
#define SQL_CHECK_DEFAULT_DATA (1 << 3)
#define SQL_CHECK_PRIMARY_KEY (1 << 4)
#define SQL_CHECK_NULLABLE (1 << 5)
#define SQL_CHECK_ALL_DATA (1 << 6)

#define SQL_CHECK_ALL (SQL_CHECK_DEFAULT_VALUES | SQL_CHECK_DEFAULT_DATA | SQL_CHECK_PRIMARY_KEY | SQL_CHECK_NULLABLE | SQL_CHECK_ALL_DATA)

/**
 * Checks if a table matches the given table specification.
 * @param db the SQLite database context
 * @param db_name the name of the attached database to use. This can be 'main', 'temp' or any attached database.
 * @param table_info the table specification
 * @param[out] error on successful exit, error will contain the number of cases where the actual table did not match the specification.
 *                   and descriptive error messages
 * @return SQLITE_OK if the table was checked successfully\n
 *         A SQLite error code otherwise
 */
int sql_check_table(sqlite3 *db, const char *db_name, const table_info_t *table_info, int check_flags, errorstream_t *error);

int sql_check_integrity(sqlite3 *db, const char *db_name, errorstream_t *error);

/**
 * Initializes a table based on the given table specification. If the table already exists, then this function is
 * equivalent to slq_check_table(). Otherwise a new table will be created based on the specification.
 *
 * @param db the SQLite database context
 * @param db_name the name of the attached database to use. This can be 'main', 'temp' or any attached database.
 * @param table_info the table specification
 * @param[out] error on successful exit, error will contain the number of errors that was encountered while initializing
                     the table and descriptive error messages.
 * @return SQLITE_OK if the table was initialized successfully\n
 *         A SQLite error code otherwise
 */
int sql_init_table(sqlite3 *db, const char *db_name, const table_info_t *table_info, errorstream_t *error);

int sql_init_stmt(sqlite3_stmt **stmt, sqlite3 *db, char *sql);

typedef void(sql_function)(sqlite3_context *, int, sqlite3_value **);

#define SQL_DETERMINISTIC 1

int sql_create_function(sqlite3 *db, const char *name, sql_function *function, int args, int flags, void *user_data, void (*destroy)(void *), errorstream_t *error);

/** @} */

#endif
