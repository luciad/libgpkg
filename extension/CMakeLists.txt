add_library ( gpkgext MODULE gpkg_ext.c )
include_directories( "${PROJECT_SOURCE_DIR}/sqlite" "${PROJECT_SOURCE_DIR}/common" )
target_link_libraries ( gpkgext gpkgcommon )

if ( ${CMAKE_C_COMPILER_ID} MATCHES "GNU" )
  target_link_libraries ( gpkgext "-Wl,--retain-symbols-file=${CMAKE_CURRENT_SOURCE_DIR}/gpkg_ext.sym" )
endif()