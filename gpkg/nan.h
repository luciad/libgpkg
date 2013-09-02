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
#ifndef GPKG_NAN_H
#define GPKG_NAN_H

#ifdef _MSC_VER
// MSVC does not provide C99 isnan; use _isnan from float.h instead.
#include <float.h>
#define gpkg_isnan(x) _isnan(x)
#else
// Default is C99 isnan
#include <math.h>
#define GPKG_NAN nan("")
#define gpkg_isnan(x) isnan(x)
#endif

#endif
