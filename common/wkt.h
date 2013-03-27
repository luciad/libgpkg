#ifndef GPB_WKT_H
#define GPB_WKT_H

#include "binstream.h"
#include "strbuf.h"
#include "geomio.h"

typedef struct wkt_writer_t {
    geom_reader_t geom_reader;
    strbuf_t strbuf;
    int type[GEOM_MAX_DEPTH];
    int children[GEOM_MAX_DEPTH];
    int offset;
} wkt_writer_t;

int wkt_writer_init( wkt_writer_t *writer );

void wkt_writer_destroy( wkt_writer_t *writer );

char* wkt_writer_getwkt( wkt_writer_t *writer );

size_t wkt_writer_length( wkt_writer_t *writer );

int wkt_read_geometry(char *data, size_t length, geom_reader_t *reader);

#endif
