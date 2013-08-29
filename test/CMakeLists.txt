find_package(Ruby REQUIRED)

get_target_property( sqlite_location sqlite_shared LOCATION )
get_target_property( gpkgext_location gpkg_ext LOCATION )

if(NOT(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR))
    configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/Gemfile ${CMAKE_CURRENT_BINARY_DIR}/Gemfile COPYONLY )
    configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/Gemfile.lock ${CMAKE_CURRENT_BINARY_DIR}/Gemfile.lock COPYONLY )
endif()

execute_process(
    COMMAND ${RUBY_EXECUTABLE} -S bundle install --gemfile=${CMAKE_CURRENT_BINARY_DIR}/Gemfile --path=${CMAKE_CURRENT_BINARY_DIR}/gems
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

file(GLOB test_scripts RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *_spec.rb)
foreach(test_script IN LISTS test_scripts)
    add_test(
        NAME ${test_script}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${RUBY_EXECUTABLE} -S bundle exec rspec ${CMAKE_CURRENT_SOURCE_DIR}/${test_script}
    )
    set_property( TEST ${test_script} PROPERTY ENVIRONMENT RUBY_SQLITE=${sqlite_location} GPKG_EXTENSION=${gpkgext_location} )
endforeach(test_script)