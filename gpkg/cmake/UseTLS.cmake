include( CheckCSourceCompiles )

function(check_tls)
  message( STATUS "Determining available thread-local storage mechanism" )
  set( ___TLS "" )
  if( NOT ___TLS )
    message( STATUS "Checking if __thread is supported" )
    check_c_source_compiles(
      "__thread int i; int main(int argc, char **argv) { return 1; }"
      TLS_USE_THREAD
    )
    set( ___TLS ${TLS_USE_THREAD} )
    if (${TLS_USE_THREAD})
      message( STATUS "Checking if __thread is supported - yes" )
      set( TLS_USE_THREAD ${TLS_USE_THREAD} PARENT_SCOPE )
    else()
      message( STATUS "Checking if __thread is supported - no" )
    endif()
  endif()

  if( NOT ___TLS )
    message( STATUS "Checking if pthread_getspecific is supported" )
    find_package ( Threads )
    if ( CMAKE_USE_PTHREADS_INIT )
      set( TLS_USE_PTHREAD 1 )
      set( ___TLS ${TLS_USE_PTHREAD} )
    endif()
    if (${TLS_USE_PTHREAD})
      message( STATUS "Checking if pthread_getspecific is supported - yes" )
      set( TLS_USE_PTHREAD ${TLS_USE_PTHREAD} PARENT_SCOPE )
    else()
      message( STATUS "Checking if pthread_getspecific is supported - no" )
    endif()
  endif()

  if( NOT ___TLS )
    message( STATUS "Checking if __declspec( thread ) is supported" )
    check_c_source_compiles(
      "__declspec( thread ) int i; int main(int argc, char **argv) { return 1; }"
      TLS_USE_DECLSPEC_THREAD
    )
    set( ___TLS ${TLS_USE_DECLSPEC_THREAD} )
    if (${TLS_USE_DECLSPEC_THREAD})
      message( STATUS "Checking if __declspec( thread ) is supported - yes" )
      set( TLS_USE_DECLSPEC_THREAD ${TLS_USE_DECLSPEC_THREAD} PARENT_SCOPE )
    else()
      message( STATUS "Checking if __declspec( thread ) is supported - no" )
    endif()
  endif()

  message( STATUS "Determining available thread-local storage mechanism - Done" )
endfunction()
