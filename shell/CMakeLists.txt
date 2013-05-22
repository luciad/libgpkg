add_executable ( gpkg shell.c )
add_definitions( -DHAVE_CONFIG_H=1 )

set ( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" )
find_package ( Readline )

include_directories( "${PROJECT_SOURCE_DIR}/sqlite" "${PROJECT_SOURCE_DIR}/common" "${PROJECT_BINARY_DIR}" )
target_link_libraries ( gpkg gpkgcommon sqlite_static )

if ( READLINE_FOUND )
  set ( HAVE_READLINE 1 )
  include_directories( "${READLINE_INCLUDE_DIR}" )
  target_link_libraries ( gpkg "${READLINE_LIBRARY}" )
endif()

configure_file ( "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.h" )
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )
