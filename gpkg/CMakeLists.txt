add_library (
  gpkg_obj OBJECT
  binstream.c
  blobio.c
  check.c
  error.c
  fp.c
  geomio.c
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
set_target_properties( gpkg_obj PROPERTIES POSITION_INDEPENDENT_CODE TRUE )

add_library ( gpkg_static STATIC $<TARGET_OBJECTS:gpkg_obj> gpkg.c )
set_target_properties(
  gpkg_static PROPERTIES
  OUTPUT_NAME "gpkg"
  PUBLIC_HEADER gpkg.h
)

if ( ${CMAKE_SYSTEM_NAME} MATCHES "Windows" )
  set_target_properties ( gpkg_static PROPERTIES PREFIX "" )
endif()

include( CheckFunctionExists )

configure_file ( "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.h" )
add_definitions( -DGPKG_HAVE_CONFIG_H )

include_directories( "${PROJECT_SOURCE_DIR}/sqlite" ${CMAKE_CURRENT_BINARY_DIR} )

add_library ( gpkg_ext MODULE $<TARGET_OBJECTS:gpkg_obj> gpkg.c )
set_target_properties( gpkg_ext PROPERTIES OUTPUT_NAME "gpkg" )

if ( WIN32 )
  set_target_properties ( gpkg_ext PROPERTIES PREFIX "" )
endif()

if ( ${CMAKE_C_COMPILER_ID} MATCHES "GNU" AND GPKG_COVERAGE )
  set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage" )
  set_target_properties( gpkg_ext PROPERTIES LINK_FLAGS "--coverage" )
endif()

if ( ${CMAKE_C_COMPILER_ID} MATCHES "GNU" OR ${CMAKE_C_COMPILER_ID} MATCHES "Clang" )
  add_definitions(-DGPKG_EXPORT=__attribute__\(\(visibility\("default"\)\)\))
  set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -std=c99 -pedantic" )
endif()

if ( MSVC )
  add_definitions(-DGPKG_CALL=__cdecl)
  target_compile_definitions( gpkg_ext PRIVATE "-DGPKG_API=__declspec\(dllexport\)" )
  set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /TP" )
endif()

if (GPKG_GEOS)
  set ( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" )
  find_package ( GEOS )
  if (GEOS_FOUND)
    message( WARNING "Using GEOS changes the license of libgpkg to LGPL 2.1" )
    add_definitions(-DGPKG_GEOM_FUNC)
    add_library ( gpkg_geos STATIC geom_geos.c )
    set_target_properties( gpkg_geos PROPERTIES POSITION_INDEPENDENT_CODE TRUE )
    include_directories( GEOS_INCLUDE_DIR )
    target_link_libraries ( gpkg_ext gpkg_geos ${GEOS_LIBRARY} )
    target_link_libraries ( gpkg_static gpkg_geos ${GEOS_LIBRARY} )
  endif()
endif()