#ifndef GPKG_SQL_H
#define GPKG_SQL_H

#include <sqlite3.h>
#include "strbuf.h"

#define SQL_NOT_NULL 0x1
#define SQL_PRIMARY_KEY 0x2

typedef enum {
    VALUE_TEXT,
    VALUE_FUNC,
    VALUE_INTEGER,
    VALUE_DOUBLE,
    VALUE_NONE
} value_type_t;

typedef struct {
    union {
        char* text;
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
	char* name;
	char* type;
    value_t default_value;
	int flags;
    char* column_constraints;
} column_info_t;

typedef struct {
    char* name;
    column_info_t * columns;
    size_t nColumns;
    value_t *rows;
    size_t nRows;
} table_info_t;

int sql_exec_for_string(sqlite3* db, char** out, char** err, char* sql, ...);

int sql_check_table(sqlite3* db, strbuf_t* err, table_info_t* table_info);

int sql_create_table(sqlite3 *db, strbuf_t *errors, table_info_t* table_info);

#endif
