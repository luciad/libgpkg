lib_LTLIBRARIES = libgpkgext.la
libgpkgext_la_SOURCES = gpkg_ext.c
libgpkgext_la_LIBADD = ../common/libgpkgcommon.la
libgpkgext_la_CPPFLAGS = -I$(top_srcdir)/common -I$(top_srcdir)/sqlite
libgpkgext_la_LDFLAGS = -export-symbols gpkg_ext.sym
