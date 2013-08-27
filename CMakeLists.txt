cmake_minimum_required( VERSION 2.8 )
project ( libgpkg )

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

