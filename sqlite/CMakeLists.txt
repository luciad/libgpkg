add_library ( sqlite STATIC sqlite3.c )
add_definitions ( -D_HAVE_SQLITE_CONFIG_H=1 )

set ( CMAKE_VERBOSE_MAKEFILE 1 )

include(CheckIncludeFile)
include(CheckFunctionExists)

check_include_file(stdint.h HAVE_STDINT_H)
check_include_file(inttype.h HAVE_INTTYPES_H)
check_include_file(utime.h HAVE_UTIME_H)

check_function_exists(gmtime_r HAVE_GMTIME_R)
check_function_exists(localtime_r HAVE_LOCALTIME_R)
check_function_exists(posix_fallocate HAVE_POSIX_FALLOCATE)
check_function_exists(usleep HAVE_USLEEP)

set ( SQLITE_ENABLE_RTREE 1 )

find_package ( Threads )
if ( CMAKE_USE_PTHREADS_INIT OR CMAKE_USE_WIN32_THREADS_INIT )
  set( SQLITE_THREADSAFE 2 )
else()
  set( SQLITE_THREADSAFE 1 )
endif()

configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.h"
)