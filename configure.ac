#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([gpkg], [1.0], [support@luciad.com])
AM_INIT_AUTOMAKE
LT_PREREQ([2.2])
LT_INIT([shared disable-static pic-only])
AC_CONFIG_MACRO_DIR([m4])
ACLOCAL_AMFLAGS = -I m4
AC_CONFIG_SRCDIR([common/gpkg.c])
AC_CONFIG_HEADERS([config.h])


# Checks for programs.
AC_PROG_CC_C99
AM_PROG_CC_C_O

# Checks for libraries.
PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h inttypes.h limits.h malloc.h stddef.h stdint.h stdlib.h string.h sys/file.h sys/ioctl.h sys/mount.h sys/param.h sys/statvfs.h sys/time.h unistd.h utime.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_UID_T
AC_TYPE_INT32_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_CHECK_MEMBERS([struct stat.st_blksize])
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_STRERROR_R
AC_CHECK_FUNCS([clock_gettime fdatasync gettimeofday localtime_r memmove memset munmap strerror strstr strtol utime])

AC_SEARCH_LIBS([dlopen], [dl])
AX_LIB_READLINE

AX_PTHREAD(
  [AC_DEFINE([HAVE_PTHREAD], [1],
    [Define if you have POSIX threads libraries and header files.])
   LIBS="$PTHREAD_LIBS $LIBS"
   CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
   CXXFLAGS="$CXXFLAGS $PTHREAD_CXXFLAGS"])


AC_CONFIG_FILES([Makefile
                 common/Makefile
                 extension/Makefile
                 shell/Makefile
                 sqlite/Makefile
                 tests/Makefile])

AC_OUTPUT
