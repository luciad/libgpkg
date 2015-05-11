cmake_minimum_required( VERSION 2.8.9 )

set( CMAKE_POSITION_INDEPENDENT_CODE true )
set( CMAKE_USER_MAKE_RULES_OVERRIDE ${CMAKE_CURRENT_SOURCE_DIR}/c_overrides.cmake )
set( CMAKE_USER_MAKE_RULES_OVERRIDE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/cxx_overrides.cmake )

project( libgpkg )

#
# Required CMake modules
#
include( CheckIncludeFile )
include( CMakeDependentOption )
include( CheckFunctionExists )

# Load version info from version.txt
file( STRINGS version.txt gpkg_VERSION )
list( GET gpkg_VERSION 0 gpkg_VERSION_MAJOR )
string( REGEX REPLACE "^.*:=" "" gpkg_VERSION_MAJOR ${gpkg_VERSION_MAJOR} )
list( GET gpkg_VERSION 1 gpkg_VERSION_MINOR )
string( REGEX REPLACE "^.*:=" "" gpkg_VERSION_MINOR ${gpkg_VERSION_MINOR} )
list( GET gpkg_VERSION 2 gpkg_VERSION_PATCH )
string( REGEX REPLACE "^.*:=" "" gpkg_VERSION_PATCH ${gpkg_VERSION_PATCH} )

set( gpkg_BUILD_VERSION "${gpkg_VERSION_MAJOR}.${gpkg_VERSION_MINOR}.${gpkg_VERSION_PATCH}" )
set( gpkg_INTERFACE_VERSION ${gpkg_VERSION_MAJOR} )

option( GPKG_TEST "Enable Testing?" OFF )
option( GPKG_COVERAGE "Enable Code coverage?" OFF )
option( GPKG_GEOS "Enable GEOS-based geometry functions?" OFF )
cmake_dependent_option( GPKG_GEOS_DL "Allow GEOS to be loaded at runtime instead of linking?" OFF "GPKG_GEOS" OFF)
option( GPKG_BOOST_GEOMETRY "Enable Boost.Geometry-based geometry functions?" OFF )

if ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mmacosx-version-min=10.5" )
endif()

find_package( Doxygen )
if( DOXYGEN_FOUND )
  configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY )
  add_custom_target(
    doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
  )
endif()

add_subdirectory( gpkg )
add_subdirectory( shell )
add_subdirectory( sqlite )

if( GPKG_TEST )
  include( CTest )
  add_subdirectory( test )
endif()
