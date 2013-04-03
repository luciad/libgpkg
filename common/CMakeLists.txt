add_library (
  gpkgcommon
  binstream.c
  geomio.c
  gpb.c
  gpkg.c
  sql.c
  strbuf.c
  wkb.c
  wkt.c
)

set ( gpkg_VERSION_MAJOR 1 )
set ( gpkg_VERSION_MINOR 0 )

configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.h"
)

include_directories( "${PROJECT_SOURCE_DIR}/sqlite" )
