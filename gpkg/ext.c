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
#include <sqlite3ext.h>
#include "gpkg.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef GPKG_EXPORT
#define GPKG_EXPORT
#endif

#ifndef GPKG_CALL
#define GPKG_CALL
#endif

GPKG_EXPORT int GPKG_CALL sqlite3_gpkg_init(sqlite3 *db, const char **pzErrMsg, const sqlite3_api_routines *pThunk) {
  return gpkg_extension_init(db, pzErrMsg, pThunk);
}

#ifdef __cplusplus
}
#endif
