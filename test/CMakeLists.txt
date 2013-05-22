find_package(Ruby REQUIRED)

get_target_property( sqlite_location sqlite_shared LOCATION )
get_target_property( gpkgext_location gpkgext LOCATION )

execute_process(
    COMMAND ${RUBY_EXECUTABLE} -S bundle install
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

file(GLOB test_scripts RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *_spec.rb)
foreach(test_script IN LISTS test_scripts)
    add_test( NAME ${test_script} COMMAND ${RUBY_EXECUTABLE} -S bundle exec rspec ${test_script} )
    set_property( TEST ${test_script} PROPERTY ENVIRONMENT RUBY_SQLITE=${sqlite_location} GPKG_EXTENSION=${gpkgext_location} )
endforeach(test_script)

