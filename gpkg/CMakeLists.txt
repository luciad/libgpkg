#
# Required CMake modules
#
set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" )

#
# Common objects for static and shared library
#
set(
  GPKG_SOURCE_FILES
  binstream.c
  blobio.c
  error.c
  fp.c
  geomio.c
  gpkg.c
  gpkg_db.c
  gpkg_geom.c
  i18n.c
  sql.c
  spatialdb.c
  spl_db.c
  spl_geom.c
  strbuf.c
  wkb.c
  wkt.c
)

#
# Check to determine how to do thread local storage
#
include( UseTLS )
check_tls()

include( UseLocale )
check_locale()

include_directories( "${PROJECT_SOURCE_DIR}/sqlite" )

#
# Geometry functions library
#
if( GPKG_GEOS )
  find_package( GEOS )
  if ( GEOS_FOUND )
    message( WARNING "GEOS is licensed under LGPL 2.1" )

    list( APPEND GPKG_SOURCE_FILES geos_context.c geos_geom_func.c geos_geom_io.c )
    include_directories( ${GEOS_INCLUDE_DIR} )
    set ( GPKG_HAVE_GEOM_FUNC "GEOS" )
    set ( GPKG_HAVE_GEOM_FUNC ${GPKG_HAVE_GEOM_FUNC} PARENT_SCOPE )
  endif()
endif()

#
# Static library of libgpkg
#
add_library( gpkg_static STATIC ${GPKG_SOURCE_FILES} )
set_target_properties( gpkg_static PROPERTIES OUTPUT_NAME "gpkg" )

#
# SQLite loadable extension version of libgpkg
#
add_library( gpkg_ext SHARED ${GPKG_SOURCE_FILES} gpkg.c )
set_target_properties( gpkg_ext PROPERTIES OUTPUT_NAME "gpkg" )

if ( GPKG_GEOS AND GEOS_FOUND )
  target_link_libraries( gpkg_ext ${GEOS_LIBRARY} )
  target_link_libraries( gpkg_static ${GEOS_LIBRARY} )
endif()

add_definitions( -DGPKG_HAVE_CONFIG_H )
configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.h" )
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

set_target_properties(
  gpkg_ext gpkg_static PROPERTIES
  VERSION ${gpkg_BUILD_VERSION}
  SOVERSION ${gpkg_INTERFACE_VERSION}
)

#
# Compiler specific settings
#
if( ${CMAKE_C_COMPILER_ID} MATCHES "GNU" OR ${CMAKE_C_COMPILER_ID} MATCHES "Clang" )
  add_definitions( -DGPKG_EXPORT=__attribute__\(\(visibility\("default"\)\)\) )
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -std=c99 -pedantic" )
endif()

if ( MSVC )
  add_definitions( -DGPKG_CALL=__cdecl )
  target_compile_definitions( gpkg_ext PRIVATE "-DGPKG_EXPORT=__declspec\(dllexport\)" )
  set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /TP" )
endif()

#
# Strip 'lib' prefix when compiling for windows
#
if ( WIN32 )
  set_target_properties(
    gpkg_static gpkg_ext PROPERTIES
    PREFIX ""
  )
endif()

#
# Enable code coverage
#
if ( ${CMAKE_C_COMPILER_ID} MATCHES "GNU" AND GPKG_COVERAGE )
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage" )
  set_target_properties( gpkg_ext PROPERTIES LINK_FLAGS "--coverage" )
endif()

if ( UNIX )
  install( TARGETS gpkg_ext LIBRARY DESTINATION lib )
  install( FILES gpkg.h DESTINATION include )
endif()