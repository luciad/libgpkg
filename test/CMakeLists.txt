include_directories( cutest )

file(READ runtest1.c run_test1)

macro(create_test in_file out_file)
  file(READ ${in_file} test_contents)
  string(REGEX MATCHALL "void Test[^" test_cases ${test_contents})
  foreach(test_case IN LISTS test_cases)
    message("Found test: ${test_case}")
  endforeach(test_case)
endmacro(create_test)

file(GLOB test_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} test*.c)
foreach(test_file IN LISTS test_files)
  create_test(${test_file} "run_${test_file}")
endforeach(test_file)

