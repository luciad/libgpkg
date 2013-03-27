#include <string.h>
#include "geomio.h"

void geom_reader_init(
        geom_reader_t *reader,
        void (*begin)(struct geom_reader_t*, geom_header_t*),
        void (*end)(struct geom_reader_t*, geom_header_t*),
        void (*coordinates)(struct geom_reader_t*, geom_header_t*, size_t point_count, double* coords)
) {
    reader->begin = begin;
    reader->end = end;
    reader->coordinates = coordinates;
}

int geom_coord_dim(geom_header_t *wkb) {
    switch (wkb->coord_type) {
		default:
		case GEOM_XY:
			return 2;
		case GEOM_XYZ:
		case GEOM_XYM:
			return 3;
		case GEOM_XYZM:
			return 4;
	}
}

char* geom_type_name(geom_header_t *wkb) {
    switch (wkb->geom_type) {
		case GEOM_POINT:
			return "ST_Point"; 
		case GEOM_LINESTRING:
			return "ST_LineString"; 
		case GEOM_POLYGON:
			return "ST_Polygon";
		case GEOM_MULTIPOINT:
			return "ST_MultiPoint";
		case GEOM_MULTILINESTRING:
			return "ST_MultiLineString";
		case GEOM_MULTIPOLYGON:
			return "ST_MultiPolygon";
		case GEOM_GEOMETRYCOLLECTION:
			return "ST_GeometryCollection";
		default:
			return NULL;
	}
}
