add_library (
  gpkgcommon
  binstream.c
  geomio.c
  gpb.c
  gpkg.c
  sql.c
  strbuf.c
  wkb.c
  wkt.c
)

set ( gpkg_VERSION_MAJOR 1 )
set ( gpkg_VERSION_MINOR 0 )

if ( ${CMAKE_C_COMPILER_ID} MATCHES "GNU" )
  set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99" )
endif()
message( "C compile flags: ${CMAKE_C_FLAGS}" )

include(CheckFunctionExists)

check_function_exists(strcasecmp HAVE_STR_CASE_CMP)
check_function_exists(stricmp HAVE_STR_ICMP)
check_function_exists(_stricmp HAVE__STR_ICMP)

check_function_exists(strncasecmp HAVE_STR_N_CASE_CMP)
check_function_exists(strnicmp HAVE_STR_N_ICMP)
check_function_exists(_strnicmp HAVE__STR_N_ICMP)

configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.h"
)

include_directories( "${PROJECT_SOURCE_DIR}/sqlite" )
