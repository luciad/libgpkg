#ifndef GPKG_SQL_H
#define GPKG_SQL_H

#include <sqlite3.h>
#include "constants.h"

typedef struct {
	char* name;
	char* type;
	int not_null;
	int primary_key;
} column_info_t;

int sql_exec_for_string(sqlite3* db, char** out, char** err, char* sql, ...);

int sql_check_table(sqlite3* db, char** err, char* table_name, column_info_t *columns, int nColumns);

#endif
