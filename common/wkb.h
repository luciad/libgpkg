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
#ifndef GPKG_WKB_H
#define GPKG_WKB_H

#include "binstream.h"
#include "geomio.h"

/**
 * \addtogroup wkb Well-known binary I/O
 * @{
 */

int wkb_read_geometry(binstream_t *stream, geom_reader_t *reader);

int wkb_read_header(binstream_t *stream, geom_header_t *header);

/** @} */

#endif
