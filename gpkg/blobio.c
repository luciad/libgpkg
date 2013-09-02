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
#include "blobio.h"

uint8_t *geom_blob_writer_getdata(geom_blob_writer_t *writer) {
  return binstream_data(&writer->wkb_writer.stream);
}

size_t geom_blob_writer_length(geom_blob_writer_t *writer) {
  return binstream_available(&writer->wkb_writer.stream);
}

geom_consumer_t *geom_blob_writer_geom_consumer(geom_blob_writer_t *writer) {
  return &writer->geom_consumer;
}
