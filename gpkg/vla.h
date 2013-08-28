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
#ifndef GPKG_VLA_H
#define GPKG_VLA_H

#ifdef GPKG_MSVC
  // MSVC does not support variable length arrays directly; use _alloca instead
  #include <malloc.h>
  #define VLA(type, name, length) type *name = (type*)_alloca((length) * sizeof(type))
#else
  // Default is C99 VLAs
  #define VLA(type, name, length) type name[length]
#endif

#endif