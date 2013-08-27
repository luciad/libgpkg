add_library (
  gpkg_obj OBJECT
  binstream.c
  check.c
  error.c
  geomio.c
  gpb.c
  gpkg.c
  tables.c
  sql.c
  strbuf.c
  wkb.c
  wkt.c
)

set_target_properties( gpkg_obj PROPERTIES POSITION_INDEPENDENT_CODE TRUE )

add_library ( gpkg_static STATIC $<TARGET_OBJECTS:gpkg_obj> )
set_target_properties( gpkg_static PROPERTIES OUTPUT_NAME "gpkg" )

if ( ${CMAKE_SYSTEM_NAME} MATCHES "Windows" )
  set_target_properties ( gpkg_static PROPERTIES PREFIX "" )
endif()

set ( gpkg_VERSION_MAJOR 0 )
set ( gpkg_VERSION_MINOR 8 )
set ( gpkg_VERSION_PATCH 0 )

if ( ${CMAKE_C_COMPILER_ID} MATCHES "GNU" )
  set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99" )
endif()

include(CheckFunctionExists)

configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.h"
)

include_directories( "${PROJECT_SOURCE_DIR}/sqlite" ${CMAKE_CURRENT_BINARY_DIR} )

add_library ( gpkg_ext MODULE $<TARGET_OBJECTS:gpkg_obj> )
set_target_properties( gpkg_ext PROPERTIES OUTPUT_NAME "gpkg" )

if ( ${CMAKE_SYSTEM_NAME} MATCHES "Windows" )
  set_target_properties ( gpkg_ext PROPERTIES PREFIX "" )
endif()

if ( ${CMAKE_C_COMPILER_ID} MATCHES "GNU" )
  target_link_libraries ( gpkg_ext "-Wl,--retain-symbols-file=${CMAKE_CURRENT_SOURCE_DIR}/gpkg.sym" )
endif()