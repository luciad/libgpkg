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

static column_info_t geopackage_contents_columns[] = {
        {"table_name", "text", N, SQL_PRIMARY_KEY | SQL_NOT_NULL, NULL},
        {"data_type", "text", N, SQL_NOT_NULL, NULL},
        {"identifier", "text", N, SQL_NOT_NULL, NULL},
        {"description", "text", T(""), SQL_NOT_NULL, NULL},
        {"last_change", "text", F("strftime('%Y-%m-%dT%H:%M:%fZ', 'now')"), SQL_NOT_NULL, NULL},
        {"min_x", "double", N, SQL_NOT_NULL, NULL},
        {"min_y", "double", N, SQL_NOT_NULL, NULL},
        {"max_x", "double", N, SQL_NOT_NULL, NULL},
        {"max_y", "double", N, SQL_NOT_NULL, NULL},
        {"srid", "integer", N, SQL_NOT_NULL, "REFERENCES spatial_ref_sys(srid)"},
        {NULL, NULL, N, 0, NULL}
};
table_info_t geopackage_contents = {
        "geopackage_contents",
        geopackage_contents_columns,
        NULL, 0
};

static column_info_t spatial_ref_sys_columns[] = {
        {"srid", "integer", N, SQL_PRIMARY_KEY, NULL},
        {"auth_name", "text", N, SQL_NOT_NULL, NULL},
        {"auth_srid", "integer", N, SQL_NOT_NULL, NULL},
        {"srtext", "text", N, SQL_NOT_NULL, NULL},
        {NULL, NULL, N, 0, NULL}
};

static value_t spatial_ref_sys_data[] = {
        I(-1), T("NONE"), I(-1), T("undefined"),
        I(0), T("NONE"), I(0), T("undefined")
};
table_info_t spatial_ref_sys = {
        "spatial_ref_sys",
        spatial_ref_sys_columns,
        spatial_ref_sys_data, 2
};

static column_info_t geometry_columns_columns[] = {
        {"f_table_name", "text", N, SQL_PRIMARY_KEY, "REFERENCES geopackage_contents(table_name)"},
        {"f_geometry_column", "text", N, SQL_PRIMARY_KEY, NULL},
        {"geometry_type", "integer", N, SQL_NOT_NULL, NULL},
        {"coord_dimension", "integer", N, SQL_NOT_NULL, NULL},
        {"srid", "integer", N, SQL_NOT_NULL, "REFERENCES spatial_ref_sys(srid)"},
        {NULL, NULL, N, 0, NULL}
};
table_info_t geometry_columns = {
        "geometry_columns",
        geometry_columns_columns,
        NULL, 0
};

static column_info_t raster_columns_columns[] = {
        {"r_table_name", "text", N, SQL_PRIMARY_KEY, "REFERENCES geopackage_contents(table_name)"},
        {"r_raster_column", "text", N, SQL_PRIMARY_KEY, NULL},
        {NULL, NULL, N, 0, NULL}
};
table_info_t raster_columns = {
        "raster_columns",
        raster_columns_columns,
        NULL, 0
};

static column_info_t tile_table_metadata_columns[] = {
        {"t_table_name", "text", N, SQL_PRIMARY_KEY, "REFERENCES geopackage_contents(table_name)"},
        {"is_times_two_zoom", "integer", I(1), SQL_NOT_NULL, NULL},
        {NULL, NULL, N, 0, NULL}
};
table_info_t tile_table_metadata = {
        "tile_table_metadata",
        tile_table_metadata_columns,
        NULL, 0
};

static column_info_t tile_matrix_metadata_columns[] = {
        {"t_table_name", "text", N, SQL_PRIMARY_KEY, "REFERENCES tile_table_metadata(t_table_name)"},
        {"zoom_level", "integer", I(0), SQL_PRIMARY_KEY, NULL},
        {"matrix_width", "integer", I(1), SQL_NOT_NULL, NULL},
        {"matrix_height", "integer", I(1), SQL_NOT_NULL, NULL},
        {"tile_width", "integer", I(256), SQL_NOT_NULL, NULL},
        {"tile_height", "integer", I(256), SQL_NOT_NULL, NULL},
        {"pixel_x_size", "double", D(1.0), SQL_NOT_NULL, NULL},
        {"pixel_y_size", "double", D(1.0), SQL_NOT_NULL, NULL},
        {NULL, NULL, N, 0, NULL}
};
table_info_t tile_matrix_metadata = {
        "tile_matrix_metadata",
        tile_matrix_metadata_columns,
        NULL, 0
};

static column_info_t tiles_columns[] = {
        {"id", "integer", N, SQL_PRIMARY_KEY, "AUTOINCREMENT"},
        {"zoom_level", "integer", I(0), SQL_NOT_NULL | SQL_UNIQUE(0), NULL},
        {"tile_column", "integer", I(0), SQL_NOT_NULL | SQL_UNIQUE(0), NULL},
        {"tile_row", "integer", I(0), SQL_NOT_NULL | SQL_UNIQUE(0), NULL},
        {"tile_data", "blob", N, SQL_NOT_NULL, NULL},
        {NULL, NULL, N, 0, NULL}
};

static column_info_t xml_metadata_columns[] = {
        {"id", "integer", N, SQL_PRIMARY_KEY, NULL},
        {"md_scope", "text", T("dataset"), SQL_NOT_NULL, NULL},
        {"md_standard_uri", "text", T("http://schemas.opengis.net/iso/19139"), SQL_NOT_NULL, NULL},
        {"mime_type", "text", T("text/xml"), SQL_NOT_NULL, NULL},
        {"metadata", "text", T(""), SQL_NOT_NULL, NULL},
        {NULL, NULL, N, 0, NULL}
};
table_info_t xml_metadata = {
        "xml_metadata",
        xml_metadata_columns,
        NULL, 0
};

static column_info_t metadata_reference_columns[] = {
        {"reference_scope", "text", N, SQL_NOT_NULL, NULL},
        {"table_name", "text", N, 0, NULL},
        {"column_name", "text", N, 0, NULL},
        {"row_id_value", "integer", N, 0, NULL},
        {"timestamp", "text", F("strftime('%Y-%m-%dT%H:%M:%fZ', 'now')"), SQL_NOT_NULL, NULL},
        {"md_file_id", "integer", N, SQL_NOT_NULL, "REFERENCES xml_metadata(id)"},
        {"md_parent_id", "integer", N, 0, "REFERENCES xml_metadata(id)"},
        {NULL, NULL, N, 0, NULL}
};
table_info_t metadata_reference = {
        "metadata_reference",
        metadata_reference_columns,
        NULL, 0
};

static column_info_t manifest_columns[] = {
        {"id", "text", N, SQL_PRIMARY_KEY, NULL},
        {"manifest", "text", N, SQL_NOT_NULL, NULL},
        {NULL, NULL, N, 0, NULL}
};
table_info_t manifest = {
        "manifest",
        manifest_columns,
        NULL, 0
};

const column_info_t tiles_table_columns[] = {
        {"id", "integer", N, SQL_PRIMARY_KEY, NULL},
        {"zoom_level", "integer", I(0), SQL_UNIQUE(1), NULL},
        {"tile_column", "integer", I(0), SQL_UNIQUE(1), NULL},
        {"tile_row", "integer", I(0), SQL_UNIQUE(1), NULL},
        {"tile_data", "blob", N, SQL_NOT_NULL, NULL},
        {NULL, NULL, N, 0, NULL}
};

const table_info_t * const tables[] = {
        &geopackage_contents,
        &spatial_ref_sys,
        &geometry_columns,
        &raster_columns,
        &tile_table_metadata,
        &tile_matrix_metadata,
        &xml_metadata,
        &metadata_reference,
        &manifest,
        NULL
};