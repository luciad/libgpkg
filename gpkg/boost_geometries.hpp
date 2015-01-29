#ifndef GPKG_BOOST_GEOMETRIES_HPP
#define GPKG_BOOST_GEOMETRIES_HPP

#include <boost/geometry/geometry.hpp>
#include <vector>

extern "C" {
#include "geomio.h"
}

namespace gpkg {
    typedef ::boost::geometry::model::point<double, 2, ::boost::geometry::cs::cartesian> Point;
    typedef ::boost::geometry::model::ring<Point, false, true> LinearRing;
    typedef ::boost::geometry::model::linestring<Point> LineString;
    typedef ::boost::geometry::model::polygon<Point, false, true> Polygon;
    typedef ::boost::geometry::model::multi_point<Point> MultiPoint;
    typedef ::boost::geometry::model::multi_linestring<LineString> MultiLineString;
    typedef ::boost::geometry::model::multi_polygon<Polygon> MultiPolygon;
    typedef ::boost::make_recursive_variant<
        Point,
        LineString,
        Polygon,
        LinearRing,
        MultiPoint,
        MultiLineString,
        MultiPolygon,
        std::vector<boost::recursive_variant_>
    >::type Geometry;
    typedef std::vector<Geometry> GeometryCollection;

    typedef ::boost::variant<
        ::boost::blank,
        Point*,
        LineString*,
        Polygon*,
        LinearRing*,
        MultiPoint*,
        MultiLineString*,
        MultiPolygon*,
        GeometryCollection*
    > GeometryPtr;

    void delete_geometry(GeometryPtr &ptr);
}

#endif
