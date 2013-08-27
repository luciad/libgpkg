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
#include "tables.h"

#define N NULL_VALUE
#define D(v) DOUBLE_VALUE(v)
#define I(v) INT_VALUE(v)
#define T(v) TEXT_VALUE(v)
#define F(v) FUNC_VALUE(v)

static column_info_t gpkg_spatial_ref_sys_columns[] = {
        {"srs_name", "text", N, SQL_NOT_NULL, NULL},
        {"srs_id", "integer", N, SQL_NOT_NULL | SQL_PRIMARY_KEY, NULL},
        {"organization", "text", N, SQL_NOT_NULL, NULL},
        {"organization_coordsys_id", "integer", N, SQL_NOT_NULL, NULL},
        {"definition", "text", N, SQL_NOT_NULL, NULL},
        {"description", "text", N, 0, NULL},
        {NULL, NULL, N, 0, NULL}
};
static value_t gpkg_spatial_ref_sys_data[] = {
        T("Undefined Cartesian"), I(-1), T("NONE"), I(-1), T("undefined"), N,
        T("Undefined Geographic"), I(0), T("NONE"), I(0), T("undefined"), N
};
table_info_t gpkg_spatial_ref_sys = {
        "gpkg_spatial_ref_sys",
        gpkg_spatial_ref_sys_columns,
        gpkg_spatial_ref_sys_data, 2
};

static column_info_t gpkg_contents_columns[] = {
        {"table_name", "text", N, SQL_NOT_NULL | SQL_PRIMARY_KEY, NULL},
        {"data_type", "text", N, SQL_NOT_NULL, NULL},
        {"identifier", "text", N, 0, NULL},
        {"description", "text", T(""), 0, NULL},
        {"last_change", "text", F("strftime('%%Y-%%m-%%dT%%H:%%M:%%fZ', 'now')"), SQL_NOT_NULL, NULL},
        {"min_x", "double", N, 0, NULL},
        {"min_y", "double", N, 0, NULL},
        {"max_x", "double", N, 0, NULL},
        {"max_y", "double", N, 0, NULL},
        {"srs_id", "integer", N, 0, "CONSTRAINT fk_srid__gpkg_spatial_ref_sys_srs_id REFERENCES gpkg_spatial_ref_sys(srs_id)"},
        {NULL, NULL, N, 0, NULL}
};
table_info_t gpkg_contents = {
        "gpkg_contents",
        gpkg_contents_columns,
        NULL, 0
};

static column_info_t gpkg_extensions_columns[] = {
        {"table_name", "text", N, SQL_UNIQUE(1), NULL},
        {"column_name", "text", N, SQL_UNIQUE(1), NULL},
        {"extension_name", "text", N, SQL_NOT_NULL | SQL_UNIQUE(1), NULL},
        {NULL, NULL, N, 0, NULL}
};
table_info_t gpkg_extensions = {
        "gpkg_extensions",
        gpkg_extensions_columns,
        NULL, 0
};

static column_info_t gpkg_geometry_columns_columns[] = {
        {"table_name", "text", N, SQL_NOT_NULL | SQL_PRIMARY_KEY,  "CONSTRAINT fk_table_name__gpkg_contents_table_name REFERENCES gpkg_contents(table_name)"},
        {"column_name", "text", N, SQL_NOT_NULL | SQL_PRIMARY_KEY, NULL},
        {"geometry_type", "text", N, SQL_NOT_NULL, NULL},
        {"srs_id", "integer", N, SQL_NOT_NULL, "CONSTRAINT fk_srs_id__gpkg_spatial_ref_sys_srs_id REFERENCES gpkg_spatial_ref_sys(srs_id)"},
        {"z", "integer", N, SQL_NOT_NULL, NULL},
        {"m", "integer", N, SQL_NOT_NULL, NULL},
        {NULL, NULL, N, 0, NULL}
};
table_info_t gpkg_geometry_columns = {
        "gpkg_geometry_columns",
        gpkg_geometry_columns_columns,
        NULL, 0
};

static column_info_t gpkg_tile_matrix_metadata_columns[] = {
        {"table_name", "text", N, SQL_NOT_NULL | SQL_PRIMARY_KEY, "CONSTRAINT fk_table_name__gpkg_contents_table_name REFERENCES gpkg_contents(table_name)"},
        {"zoom_level", "integer", N, SQL_NOT_NULL | SQL_PRIMARY_KEY, NULL},
        {"matrix_width", "integer", N, SQL_NOT_NULL, NULL},
        {"matrix_height", "integer", N, SQL_NOT_NULL, NULL},
        {"tile_width", "integer", N, SQL_NOT_NULL, NULL},
        {"tile_height", "integer", N, SQL_NOT_NULL, NULL},
        {"pixel_x_size", "double", N, SQL_NOT_NULL, NULL},
        {"pixel_y_size", "double", N, SQL_NOT_NULL, NULL},
        {NULL, NULL, N, 0, NULL}
};
table_info_t gpkg_tile_matrix_metadata = {
        "gpkg_tile_matrix_metadata",
        gpkg_tile_matrix_metadata_columns,
        NULL, 0
};

const column_info_t tiles_table_columns[] = {
        {"id", "integer", N, SQL_PRIMARY_KEY | SQL_AUTOINCREMENT, NULL},
        {"zoom_level", "integer", N, SQL_NOT_NULL | SQL_UNIQUE(1), NULL},
        {"tile_column", "integer", N, SQL_NOT_NULL | SQL_UNIQUE(1), NULL},
        {"tile_row", "integer", N, SQL_NOT_NULL | SQL_UNIQUE(1), NULL},
        {"tile_data", "blob", N, SQL_NOT_NULL, NULL},
        {NULL, NULL, N, 0, NULL}
};

static column_info_t gpkg_data_columns_columns[] = {
        {"table_name", "text", N, SQL_NOT_NULL | SQL_PRIMARY_KEY, "CONSTRAINT fk_table_name__gpkg_contents_table_name REFERENCES gpkg_contents(table_name)"},
        {"column_name", "text", N, SQL_NOT_NULL | SQL_PRIMARY_KEY, NULL},
        {"name", "text", N, 0, NULL},
        {"title", "text", N, 0, NULL},
        {"description", "text", N, 0, NULL},
        {"mime_type", "text", N, 0, NULL},
        {NULL, NULL, N, 0, NULL}
};
table_info_t gpkg_data_columns = {
        "gpkg_data_columns",
        gpkg_data_columns_columns,
        NULL, 0
};

static column_info_t gpkg_metadata_columns[] = {
        {"id", "integer", N, SQL_NOT_NULL | SQL_AUTOINCREMENT | SQL_PRIMARY_KEY, NULL},
        {"md_scope", "text", T("dataset"), SQL_NOT_NULL, NULL},
        {"md_standard_uri", "text", T("http://schemas.opengis.net/iso/19139"), SQL_NOT_NULL, NULL},
        {"mime_type", "text", T("text/xml"), SQL_NOT_NULL, NULL},
        {"metadata", "text", T(""), SQL_NOT_NULL, NULL},
        {NULL, NULL, N, 0, NULL}
};
table_info_t gpkg_metadata = {
        "gpkg_metadata",
        gpkg_metadata_columns,
        NULL, 0
};

static column_info_t gpkg_metadata_reference_columns[] = {
        {"reference_scope", "text", N, SQL_NOT_NULL, NULL},
        {"table_name", "text", N, 0, NULL},
        {"column_name", "text", N, 0, NULL},
        {"row_id_value", "integer", N, 0, NULL},
        {"timestamp", "text", F("strftime('%%Y-%%m-%%dT%%H:%%M:%%fZ', 'now')"), SQL_NOT_NULL, NULL},
        {"md_file_id", "integer", N, SQL_NOT_NULL, "CONSTRAINT fk_file_id__metadata_id REFERENCES gpkg_metadata(id)"},
        {"md_parent_id", "integer", N, 0, "CONSTRAINT fk_parent_id__metadata_id REFERENCES gpkg_metadata(id)"},
        {NULL, NULL, N, 0, NULL}
};
table_info_t gpkg_metadata_reference = {
        "gpkg_metadata_reference",
        gpkg_metadata_reference_columns,
        NULL, 0
};

const table_info_t * const tables[] = {
        &gpkg_contents,
        &gpkg_extensions,
        &gpkg_spatial_ref_sys,
        &gpkg_data_columns,
        &gpkg_metadata,
        &gpkg_metadata_reference,
        &gpkg_geometry_columns,
        &gpkg_tile_matrix_metadata,
        NULL
};
