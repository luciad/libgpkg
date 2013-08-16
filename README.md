# Description
A SQLite 3 extension that provides a minimal [OGC GeoPackage](http://www.ogcnetwork.net/geopackage) implementation.

GeoPackage is an open, standards-based, application and platform independent, and self-describing file format for geodata based on SQLite.

[Luciad](http://www.luciad.com) is actively participating in the GeoPackage Standards Working Group to define this standard. We are releasing this library under the liberal [Apache Software License](https://www.apache.org/licenses/LICENSE-2.0) to promote widespread adoption of this new format.

Luciad is using this library in its LuciadLightspeed and LuciadMobile products to handle GeoPackage data. Both products provide
a rich set of components for geospatial situational awareness. See [www.luciad.com](http://www.luciad.com) for more information. 

A free GeoPackage viewer on Android, based on LuciadMobile, is available at [demo.luciad.com/GeoPackage](http://demo.luciad.com/GeoPackage/).

# License
libgpkg is distributed under the [Apache Software License](https://www.apache.org/licenses/LICENSE-2.0) version 2.0.

# Usage
libgpkg can be loaded into SQLite using the [sqlite3\_load\_extension](http://sqlite.org/c3ref/load_extension.html) C function or using the [load\_extension](http://sqlite.org/lang_corefunc.html#load_extension) SQL function. Once loaded libgpkg extends SQLite with the function listed below. These function can be used just like any of the core functions that SQLite provides.

## Standard Functions
- ST\_MinX, ST\_MaxX, ST\_MinY, ST\_MaxY, ST\_MinZ, ST\_MaxZ, ST\_MinM, ST\_MaxM.
- ST\_SRID
- ST\_IsValid
- ST\_IsMeasured
- ST\_Is3d
- ST\_CoordDim
- ST\_GeometryType
- ST\_AsBinary, ST\_GeomFromWKB, ST\_WKBToSQL
- ST\_AsText, ST\_GeomFromText, ST\_WKBFromText, ST\_WKTToSQL

## Non-Standard Functions
- CheckGpkg
- InitGpkg
- AddGeometryColumn
- CreateTilesTable

# Compilation

- Install CMake 3.8 or newer. CMake can be downloaded from www.cmake.org or installed using
  your systems package manager.
- Run 'cmake .' in the root of directory of the project to generate the build scripts for your system.
- Build the project using the generated build scripts.
- The build scripts will generate a number of binaries
    - shell/gpkg: a modified version of the SQLite 3 command-line shell that autoloads the GeoPackage extension. This is a standalone binary that has been statically linked with SQLite 3 and the GeoPackage extension.
    - extension/libgpkgext.so: a dynamically loadable SQLite 3 extension that provides the GeoPackage functionality. This extension library can be used with any SQLite 3 that supports extension loading.
