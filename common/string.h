#ifndef GPKG_STRING_H
#define GPKG_STRING_H

#include <string.h>

#if defined(WIN32) || defined(WIN64)
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif

#endif