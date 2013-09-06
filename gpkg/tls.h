#ifndef GPKG_TLS_H
#define GPKG_TLS_H

#if defined(GPKG_HAVE_THREAD)
  #define GPKG_TLS_KEY(name) static __thread void *name;
  #define GPKG_TLS_KEY_CREATE(name) do {} while(0)
  #define GPKG_TLS_GET(key) key
  #define GPKG_TLS_SET(key, value) key = value
#elif defined(_WIN32)
  #define GPKG_TLS_KEY(name) static __declspec( thread ) void *name;
  #define GPKG_TLS_KEY_CREATE(name) do {} while(0)
  #define GPKG_TLS_GET(key) key
  #define GPKG_TLS_SET(key, value) key = value
#elif defined(GPKG_HAVE_PTHREAD)
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
  #error "Thread local storage not available."
  #define GPKG_TLS_KEY(name) static void *name;
  #define GPKG_TLS_KEY_CREATE(name) do {} while(0)
  #define GPKG_TLS_GET(key) key
  #define GPKG_TLS_SET(key, value) key = value
#endif

#endif

