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
#ifndef GPKG_DEBUG_H
#define GPKG_DEBUG_H

#ifdef __ANDROID__

#include <android/log.h>

#ifdef DEBUG
#define gpkg_d(...)
#else
#define gpkg_d(...) __android_log_print(ANDROID_LOG_DEBUG, "GeoPackage", __VA_ARGS__)
#endif

#define gpkg_i(...) __android_log_print(ANDROID_LOG_INFO, "GeoPackage", __VA_ARGS__)
#define gpkg_e(...) __android_log_print(ANDROID_LOG_ERROR, "GeoPackage", __VA_ARGS__)

#else

#include <stdio.h>

#ifdef DEBUG
#define gpkg_d(...)
#else
#define gpkg_d(...) fprintf(stdout, __VA_ARGS__)
#endif

#define gpkg_i(...) fprintf(stdout, __VA_ARGS__)
#define gpkg_e(...) fprintf(stderr, __VA_ARGS__)

#endif

#endif
