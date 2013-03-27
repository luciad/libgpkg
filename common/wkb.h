#ifndef GPKG_WKB_H
#define GPKG_WKB_H

#include "binstream.h"
#include "geomio.h"

int wkb_read_geometry(binstream_t *stream, geom_reader_t *reader);

int wkb_read_header(binstream_t *stream, geom_header_t *header);

#endif
