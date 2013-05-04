include_directories( cutest )

macro(create_test in_file out_file)
  file(READ ${in_file} test_contents)
  string(REGEX MATCHALL "void Test[^\\(]+\\([\\t ]*CuTest[ ]*\\*[^\\)]+\\)" test_cases ${test_contents})

  file(WRITE ${out_file}
"#include <stdio.h>
#include <stdlib.h>

#include \"cutest/CuTest.h\"

")

  foreach(test_case IN LISTS test_cases)
    file(APPEND ${out_file} "extern ${test_case};\n")
  endforeach(test_case)

  file(APPEND ${out_file}
"
int main(void) {
  CuString *output = CuStringNew();
  CuSuite* suite = CuSuiteNew();

")

  foreach(test_case IN LISTS test_cases)
    string(REGEX MATCH "Test[^\\(]+" test_case_name ${test_case})
    file(APPEND ${out_file} "  SUITE_ADD_TEST(suite, ${test_case_name});\n")
  endforeach(test_case)

  file(APPEND ${out_file}
"
  CuSuiteRun(suite);
  CuSuiteSummary(suite, output);
  CuSuiteDetails(suite, output);
  printf(\"%s\\n\", output->buffer);
  CuStringDelete(output);
  CuSuiteDelete(suite);

  return suite->failCount == 0 ? 0 : 1;
}
")
endmacro(create_test)

file(GLOB test_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} test*.c)
foreach(test_file IN LISTS test_files)
  string(REGEX MATCH "test[^\\.]+" test_name ${test_file})
  string(REGEX REPLACE "^test" "" test_name ${test_name})

  set(run_test_source "run_${test_name}.c")
  create_test(${test_file} ${run_test_source})

  set(run_test "run_${test_name}")
  add_executable(${run_test} ${run_test_source} ${test_file} cutest/CuTest.c)
  add_test(NAME ${test_name} COMMAND ${run_test})
endforeach(test_file)

