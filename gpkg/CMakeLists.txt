#
# Required CMake modules
#
set ( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" )

#
# Common objects for static and shared library
#
set(
  GPKG_SOURCE_FILES
  binstream.c
  blobio.c
  check.c
  error.c
  fp.c
  geomio.c
  gpkg.c
  gpkg_db.c
  gpkg_geom.c
  sql.c
  spatialdb.c
  spl_db.c
  spl_geom.c
  strbuf.c
  wkb.c
  wkt.c
)

add_definitions( -DGPKG_HAVE_CONFIG_H )

#
# Check to determine how to do thread local storage
#
include( UseTLS )
check_tls()

configure_file ( "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.h" )

include_directories( "${PROJECT_SOURCE_DIR}/sqlite" ${CMAKE_CURRENT_BINARY_DIR} )

#
# Static library of libgpkg
#
add_library ( gpkg_static STATIC ${GPKG_SOURCE_FILES} )
set_target_properties(
  gpkg_static PROPERTIES
  OUTPUT_NAME "gpkg"
  PUBLIC_HEADER gpkg.h
)

#
# SQLite loadable extension version of libgpkg
#
add_library ( gpkg_ext MODULE ${GPKG_SOURCE_FILES} gpkg.c )
set_target_properties( gpkg_ext PROPERTIES OUTPUT_NAME "gpkg" )

#
# Geometry functions library
#
set(GPKG_GEOM_FUNC "")

if (NOT GPKG_GEOM_FUNC)
  if (GPKG_GEOS)
    find_package ( GEOS )
    if (GEOS_FOUND)
      message( WARNING "GEOS is licensed under LGPL 2.1" )

      add_library ( gpkg_geom STATIC geom_func_geos.c )
      include_directories( GEOS_INCLUDE_DIR )

      set(GPKG_GEOM_FUNC "GEOS")
    endif()
  endif()
endif()

if(NOT GPKG_GEOM_FUNC)
  add_library ( gpkg_geom STATIC geom_func_null.c )
  set(GPKG_GEOM_FUNC "NONE")
endif()

target_link_libraries ( gpkg_ext gpkg_geom ${GEOS_LIBRARY} )
target_link_libraries ( gpkg_static gpkg_geom ${GEOS_LIBRARY} )

#
# Compiler specific settings
#
if ( ${CMAKE_C_COMPILER_ID} MATCHES "GNU" OR ${CMAKE_C_COMPILER_ID} MATCHES "Clang" )
  add_definitions( -DGPKG_EXPORT=__attribute__\(\(visibility\("default"\)\)\) )
  set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -fvisibility=hidden -std=c99 -pedantic" )
endif()

if ( MSVC )
  add_definitions(-DGPKG_CALL=__cdecl)
  target_compile_definitions( gpkg_ext PRIVATE "-DGPKG_API=__declspec\(dllexport\)" )
  set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /TP" )
endif()

#
# Strip 'lib' prefix when compiling for windows
#
if ( WIN32 )
  set_target_properties ( gpkg_static PROPERTIES PREFIX "" )
  set_target_properties ( gpkg_ext PROPERTIES PREFIX "" )
endif()

#
# Enable code coverage
#
if ( ${CMAKE_C_COMPILER_ID} MATCHES "GNU" AND GPKG_COVERAGE )
  set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage" )
  set_target_properties( gpkg_ext PROPERTIES LINK_FLAGS "--coverage" )
endif()