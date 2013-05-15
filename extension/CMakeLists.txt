add_library ( gpkgext SHARED gpkg_ext.c )
include_directories( "${PROJECT_SOURCE_DIR}/sqlite" "${PROJECT_SOURCE_DIR}/common" )
target_link_libraries ( gpkgext gpkgcommon )

if ( ${CMAKE_C_COMPILER_ID} MATCHES "GNU" )
  target_link_libraries ( gpkgext "-Wl,--retain-symbols-file=gpkg_ext.sym" )
endif()