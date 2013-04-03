add_library ( gpkgext SHARED gpkg_ext.c )
include_directories( "${PROJECT_SOURCE_DIR}/sqlite" "${PROJECT_SOURCE_DIR}/common" )
target_link_libraries ( gpkgext gpkgcommon )