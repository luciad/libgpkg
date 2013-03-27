#ifndef GPKG_GEOMIO_H
#define GPKG_GEOMIO_H

#include <stdint.h>

#define GEOM_POINT 1
#define GEOM_LINESTRING 2
#define GEOM_POLYGON 3
#define GEOM_MULTIPOINT 4
#define GEOM_MULTILINESTRING 5
#define GEOM_MULTIPOLYGON 6
#define GEOM_GEOMETRYCOLLECTION 7

#define GEOM_LINEARRING 900

#define GEOM_BE 0
#define GEOM_LE 1

#define GEOM_MAX_DEPTH 25

typedef enum {
    GEOM_XY = 0,
    GEOM_XYZ = 1000,
    GEOM_XYM = 2000,
    GEOM_XYZM = 3000
} coord_type_t;

typedef struct {
    uint32_t geom_type;
    coord_type_t coord_type;
    uint32_t coord_size;
} geom_header_t;

typedef struct geom_reader_t {
    void (*begin)(struct geom_reader_t *reader, geom_header_t *header);
    void (*end)(struct geom_reader_t *reader, geom_header_t *header);
    void (*coordinates)(struct geom_reader_t *reader, geom_header_t *header, size_t point_count, double *coords);
} geom_reader_t;

void geom_reader_init(
        geom_reader_t *reader,
        void (*begin)(geom_reader_t *, geom_header_t *),
        void (*end)(geom_reader_t *, geom_header_t *),
        void (*coordinates)(geom_reader_t *, geom_header_t *, size_t point_count, double *coords)
);

int geom_coord_dim(geom_header_t *wkb);

char* geom_type_name(geom_header_t *wkb);

#endif
