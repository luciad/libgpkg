#ifndef GPKG_TABLES_H
#define GPKG_TABLES_H

#include <stdlib.h>
#include "sql.h"

column_info_t geopackage_contents_columns[] = {
        {"table_name", "text", {NULL}, SQLITE_NULL, SQL_PRIMARY_KEY | SQL_NOT_NULL, NULL},
        {"data_type", "text", {NULL}, SQLITE_NULL, SQL_NOT_NULL, NULL},
        {"identifier", "text", {NULL}, SQLITE_NULL, SQL_NOT_NULL, NULL},
        {"description", "text", {""}, SQLITE_TEXT, SQL_NOT_NULL, NULL},
        {"last_change", "text", {"strftime('%Y-%m-%dT%H:%M:%fZ', CURRENT_TIMESTAMP)"}, SQLITE_TEXT, SQL_NOT_NULL, NULL},
        {"min_x", "double", {.double_value = -180.0}, SQLITE_FLOAT, SQL_NOT_NULL, NULL},
        {"min_y", "double", {.double_value = -90.0}, SQLITE_FLOAT, SQL_NOT_NULL, NULL},
        {"max_x", "double", {.double_value = 180.0}, SQLITE_FLOAT, SQL_NOT_NULL, NULL},
        {"max_y", "double", {.double_value = 90.0}, SQLITE_FLOAT, SQL_NOT_NULL, NULL},
        {"srid", "integer", {.int_value = 0}, SQLITE_INTEGER, SQL_NOT_NULL, "REFERENCES spatial_ref_sys(srid)"}
};
table_info_t geopackage_contents = {"geopackage_contents", geopackage_contents_columns, 10};

column_info_t spatial_ref_sys_columns[] = {
        {"srid", "integer", {NULL}, SQLITE_NULL, SQL_PRIMARY_KEY, NULL},
        {"auth_name", "text", {NULL}, SQLITE_NULL, SQL_NOT_NULL, NULL},
        {"auth_srid", "integer", {NULL}, SQLITE_NULL, SQL_NOT_NULL, NULL},
        {"srtext", "text", {NULL}, SQLITE_NULL, SQL_NOT_NULL, NULL}
};
table_info_t spatial_ref_sys = {"spatial_ref_sys", spatial_ref_sys_columns, 4};

table_info_t *tables[] = {
        &geopackage_contents,
        &spatial_ref_sys,
        NULL
};

#endif