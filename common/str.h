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