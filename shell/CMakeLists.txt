add_executable ( gpkg_shell shell.c )
set_target_properties ( gpkg_shell PROPERTIES OUTPUT_NAME "gpkg" )
add_definitions( -DHAVE_CONFIG_H=1 )

set ( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" )
find_package ( Readline )

include_directories( "${PROJECT_SOURCE_DIR}/sqlite" "${PROJECT_SOURCE_DIR}/gpkg" "${PROJECT_BINARY_DIR}" )
target_link_libraries ( gpkg_shell gpkg_static sqlite_static )

if ( READLINE_FOUND )
  set ( HAVE_READLINE 1 )
  include_directories( "${READLINE_INCLUDE_DIR}" )
  target_link_libraries ( gpkg_shell "${READLINE_LIBRARY}" )
endif()

configure_file ( "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.h" )
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

if ( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
  install(
    TARGETS gpkg_shell
    RUNTIME DESTINATION "bin"
  )
endif()