TESTS = check_gpkg
check_PROGRAMS = check_gpkg
check_gpkg_SOURCES = check_gpkg.c
check_gpkg_CFLAGS = @CHECK_CFLAGS@ -I$(top_srcdir)/common -I$(top_srcdir)/sqlite
check_gpkg_LDADD = ../common/libgpkgcommon.la ../sqlite/libsqlite.la @CHECK_LIBS@