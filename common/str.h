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
#ifndef GPKG_STRING_H
#define GPKG_STRING_H

#include <string.h>

#if defined(HAVE_STR_CASE_CMP)
#define STRICMP strcasecmp
#elif defined(HAVE_STR_ICMP)
#define STRNICMP stricmp
#elif defined(HAVE__STR_ICMP)
#define STRNICMP _stricmp
#else
#define STRICMP strcasecmp
#endif

#if defined(HAVE_STR_N_CASE_CMP)
#define STRNICMP strncasecmp
#elif defined(HAVE_STR_N_ICMP)
#define STRNICMP strnicmp
#elif defined(HAVE__STR_N_ICMP)
#define STRNICMP _strnicmp
#else
#define STRNICMP strncasecmp
#endif

#endif