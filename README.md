= Description
A SQLite 3 extension that provides a minimal GeoPackage implementation.

= Compilation

- Install CMake 3.8 or newer. CMake can be downloaded from www.cmake.org or installed using
  your systems package manager.
- Run 'cmake .' in the root of directory of the project to generate the build scripts for your system.
- Build the project using the generated build scripts.
- The build scripts will generate a number of binaries
  - shell/gpkg: a modified version of the SQLite 3 command-line shell that autoloads the GeoPackage extension. This is a standalone binary that has been statically linked with SQLite 3 and the GeoPackage extension.
  - extension/libgpkgext.so: a dynamically loadable SQLite 3 extension that provides the GeoPackage functionality. This extension library can be used with any SQLite 3 that supports extension loading.
