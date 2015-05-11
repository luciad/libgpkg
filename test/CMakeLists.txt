#
# libgpkg unit tests
#
# The libgpkg unit test suite consists of rspec tests that excercise libgpkg through the SQL/CLI. The tests use
# a custom sqlite ruby binding (see sqlite.rb) to ensure that the tests are run using the sqlite3 binary from
# '../sqlite'.
#
get_target_property( sqlite_location sqlite_shared LOCATION )
get_target_property( gpkgext_location gpkg_ext LOCATION )

#
# Copy the Gemfile when making an out of source build
#
if( NOT( CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR ) )
  configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/Gemfile ${CMAKE_CURRENT_BINARY_DIR}/Gemfile COPYONLY )
endif()

#
# Ensure the required gems are available
#
execute_process(
  COMMAND ruby -S bundle install --gemfile=${CMAKE_CURRENT_BINARY_DIR}/Gemfile --path=${CMAKE_CURRENT_BINARY_DIR}/gems
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

#
# Generate a cmake test for each (libgpkg entry point, spec file) combination.
#
set(entry_points gpkg gpkg_spl3 gpkg_spl4)

file(GLOB test_scripts RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *_spec.rb)
foreach(entry_point IN LISTS entry_points)
  foreach(test_script IN LISTS test_scripts)
    set(test_name ${entry_point}_${test_script})
    add_test(
      NAME ${test_name}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMAND ruby -S bundle exec rspec ${CMAKE_CURRENT_SOURCE_DIR}/${test_script}
    )
    set_property(
      TEST ${test_name}
      PROPERTY ENVIRONMENT
      RUBY_SQLITE=${sqlite_location}
      GPKG_EXTENSION=${gpkgext_location}
      GPKG_ENTRY_POINT=${entry_point}
    )
    if ( GPKG_GEOM_FUNC )
      set_property(
        TEST ${test_name}
        APPEND
        PROPERTY ENVIRONMENT
        GPKG_GEOM_FUNC=TRUE
      )
    endif()
  endforeach(test_script)
endforeach(entry_point)
