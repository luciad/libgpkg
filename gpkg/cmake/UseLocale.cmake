include( CheckCSourceCompiles )

function(check_locale)
  message( STATUS "Determining available locale mechanism" )
  set( ___LOCALE "" )

  if( NOT ___LOCALE )
    message( STATUS "Checking if _create_locale is supported" )
    check_include_file(locale.h HAVE_LOCALE_H)
    if ( HAVE_LOCALE_H )
      check_c_source_compiles(
        "#include <locale.h>
        int main(int argc, char **argv) {
          _locale_t locale = _create_locale( LC_ALL, \"C\" );
          return 1;
        }"
        LOCALE_USE__CREATE_LOCALE
      )
    endif()

    set( ___LOCALE ${LOCALE_USE__CREATE_LOCALE} )
    if (${LOCALE_USE__CREATE_LOCALE})
      message( STATUS "Checking if _create_locale is supported - yes" )
      set( LOCALE_USE__CREATE_LOCALE ${LOCALE_USE__CREATE_LOCALE} PARENT_SCOPE )
    else()
      message( STATUS "Checking if _create_locale is supported - no" )
    endif()
  endif()

  if( NOT ___LOCALE )
    message( STATUS "Checking if newlocale is supported" )
    check_include_file(locale.h HAVE_LOCALE_H)
    check_include_file(xlocale.h HAVE_XLOCALE_H)
    if( HAVE_LOCALE_H AND HAVE_XLOCALE_H  )
      check_c_source_compiles(
        "#define _POSIX_C_SOURCE 200809L
        #define _GNU_SOURCE
        #include <locale.h>
        #include <xlocale.h>
        int main(int argc, char **argv) {
          locale_t locale = newlocale( LC_ALL, \"C\", NULL );
          return 1;
        }"
        LOCALE_USE_NEWLOCALE
      )
    elseif ( HAVE_LOCALE_H )
      check_c_source_compiles(
        "#define _POSIX_C_SOURCE 200809L
        #define _GNU_SOURCE
        #include <locale.h>
        int main(int argc, char **argv) {
          locale_t locale = newlocale( LC_ALL, \"C\", NULL );
          return 1;
        }"
        LOCALE_USE_NEWLOCALE
      )
    elseif( HAVE_XLOCALE_H )
      check_c_source_compiles(
        "#define _POSIX_C_SOURCE 200809L
        #define _GNU_SOURCE
        #include <xlocale.h>
        int main(int argc, char **argv) {
          locale_t locale = newlocale( LC_ALL, \"C\", NULL );
          return 1;
        }"
        LOCALE_USE_NEWLOCALE
      )
    endif()

    set( ___LOCALE ${LOCALE_USE_NEWLOCALE} )
    if (${LOCALE_USE_NEWLOCALE})
      message( STATUS "Checking if newlocale is supported - yes" )
      set( LOCALE_USE_NEWLOCALE ${LOCALE_USE_NEWLOCALE} PARENT_SCOPE )
      set( HAVE_LOCALE_H ${HAVE_LOCALE_H} PARENT_SCOPE )
      set( HAVE_XLOCALE_H ${HAVE_XLOCALE_H} PARENT_SCOPE )
    else()
      message( STATUS "Checking if newlocale is supported - no" )
    endif()
  endif()

  if( NOT ___LOCALE )
    message( STATUS "Checking if setlocale is supported" )
    check_c_source_compiles(
      "#include <locale.h>
      int main(int argc, char **argv) {
        setlocale( LC_ALL, \"C\" );
        return 1;
      }"
      LOCALE_USE_SET_LOCALE
    )

    set( ___LOCALE ${LOCALE_USE_SET_LOCALE} )
    if (${LOCALE_USE_SET_LOCALE})
      message( STATUS "Checking if setlocale is supported - yes" )
      set( LOCALE_USE_SET_LOCALE ${LOCALE_USE_SET_LOCALE} PARENT_SCOPE )
    else()
      message( STATUS "Checking if setlocale is supported - no" )
    endif()
  endif()

  message( STATUS "Determining available locale mechanism - Done" )
endfunction()
