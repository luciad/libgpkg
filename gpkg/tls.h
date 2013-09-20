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
#ifndef GPKG_TLS_H
#define GPKG_TLS_H

#ifdef GPKG_HAVE_CONFIG_H
#include "config.h"
#endif

#if defined(TLS_USE_THREAD)
#define GPKG_TLS_KEY(name) static __thread void *name;
#define GPKG_TLS_KEY_CREATE(name) do {} while(0)
#define GPKG_TLS_GET(key) key
#define GPKG_TLS_SET(key, value) key = value
#elif defined(TLS_USE_DECLSPEC_THREAD)
#define GPKG_TLS_KEY(name) static __declspec( thread ) void *name;
#define GPKG_TLS_KEY_CREATE(name) do {} while(0)
#define GPKG_TLS_GET(key) key
#define GPKG_TLS_SET(key, value) key = value
#elif defined(TLS_USE_PTHREAD)
#include <pthread.h>
#define GPKG_TLS_KEY(name)\
    static pthread_key_t name;\
    static pthread_once_t name##_once = PTHREAD_ONCE_INIT;\
    static void name##_init_once() {\
      pthread_key_create(&name, NULL);\
    }
#define GPKG_TLS_KEY_CREATE(name) pthread_once(&name##_once, name##_init_once)
#define GPKG_TLS_GET(key) pthread_getspecific(key)
#define GPKG_TLS_SET(key, value) pthread_setspecific(key, value)
#else
#error "Thread local storage is not supported"
#define GPKG_TLS_KEY(name) static void *name;
#define GPKG_TLS_KEY_CREATE(name) do {} while(0)
#define GPKG_TLS_GET(key) key
#define GPKG_TLS_SET(key, value) key = value
#endif

#endif

