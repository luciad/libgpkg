#ifndef GPKG_SQL_H
#define GPKG_SQL_H

#include <sqlite3.h>
#include "strbuf.h"

#define SQL_NOT_NULL 0x1
#define SQL_PRIMARY_KEY 0x2

typedef struct {
	char* name;
	char* type;
    union {
        char* text_value;
        double double_value;
        int int_value;
    } default_value;
    int default_value_type;
	int flags;
    char* column_constraints;
} column_info_t;

typedef struct {
    char* name;
    column_info_t * columns;
    size_t nColumns;
} table_info_t;

int sql_exec_for_string(sqlite3* db, char** out, char** err, char* sql, ...);

int sql_check_table(sqlite3* db, strbuf_t* err, table_info_t* table_info);

int sql_create_table(sqlite3 *db, strbuf_t *errors, table_info_t* table_info);

#endif
