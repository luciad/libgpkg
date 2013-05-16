if ( ${CMAKE_C_COMPILER_ID} MATCHES "GNU" )
  set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99" )
endif()

macro(create_test_runner out_file test_name)
  file(WRITE ${out_file}
"#include \"unittest.h\"

extern void ${test_name}();

int main(void) {
  unittest_init();
  ${test_name}();
  return 0;
}
")
endmacro(create_test_runner)

add_library (
  unittest STATIC
  unittest.c
)
set_target_properties( unittest PROPERTIES POSITION_INDEPENDENT_CODE TRUE )
target_link_libraries(unittest gpkgcommon sqlite)

file(GLOB test_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} test*.c)
foreach(test_file IN LISTS test_files)
  string(REGEX MATCH "test[^\\.]+" test_name ${test_file})
  string(REGEX REPLACE "^test" "" test_name ${test_name})

  file(READ ${test_file} test_contents)
  string(REGEX MATCHALL "void Test[^\\(]+\\(\\)" test_cases ${test_contents})
  foreach(test_case IN LISTS test_cases)
    string(REGEX MATCH "Test[^\\(]+" test_case_name ${test_case})

    set(run_test_source "run_${test_name}_${test_case_name}.c")
    create_test_runner(${run_test_source} ${test_case_name})

    set(run_test "run_${test_name}_${test_case_name}")

    add_executable(${run_test} ${run_test_source} ${test_file})
    target_link_libraries(${run_test} unittest)
    add_test(NAME ${test_name}_${test_case_name} COMMAND ${run_test})
  endforeach(test_case)
endforeach(test_file)

include_directories( "../common" "../sqlite" )