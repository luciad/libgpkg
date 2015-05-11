#ifndef GPKG_DYNLIB_H
#define GPKG_DYNLIB_H

#if defined(_WIN32) || defined(WIN32) || defined(__MINGW32__)

#include <Windows.h>

#define dynlib HMODULE
#define dynlib_open(path) LoadLibrary(path)
#define dynlib_sym(handle,symbol) GetProcAddress(handle,symbol)
#define dynlib_close(handle) FreeLibrary(handle)

#else

#include <dlfcn.h>

#define dynlib void*
#define dynlib_open(path) dlopen(path, RTLD_LAZY | RTLD_LOCAL)
#define dynlib_sym(handle,symbol) dlsym(handle,symbol)
#define dynlib_close(handle) dlclose(handle)
#define dynlib_error(handle) dlerror()

#endif

#endif
