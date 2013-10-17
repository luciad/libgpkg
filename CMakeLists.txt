cmake_minimum_required( VERSION 2.8.7 )
project ( libgpkg )

#
# Required CMake modules
#
include(CheckIncludeFile)
include(CheckFunctionExists)

set ( gpkg_VERSION_MAJOR 0 )
set ( gpkg_VERSION_MINOR 9 )
set ( gpkg_VERSION_PATCH 4 )
set ( gpkg_BUILD_VERSION "${gpkg_VERSION_MAJOR}.${gpkg_VERSION_MINOR}.${gpkg_VERSION_PATCH}" )
set ( gpkg_INTERFACE_VERSION ${gpkg_VERSION_MAJOR} )

option(GPKG_TEST "Enable Testing?" OFF)
option(GPKG_COVERAGE "Enable Code coverage?" OFF)
option(GPKG_GEOS "Enable GEOS-based geometry functions?" OFF)

if ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
  set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mmacosx-version-min=10.5" )
endif()

find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                    COMMENT "Generating API documentation with Doxygen" VERBATIM
  )
endif(DOXYGEN_FOUND)

add_subdirectory( gpkg )
add_subdirectory( shell )
add_subdirectory( sqlite )

if(GPKG_TEST)
  include( CTest )
  add_subdirectory( test )
endif()
