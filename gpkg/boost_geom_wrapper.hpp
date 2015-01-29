#ifndef BOOST_GEOM_WRAPPER_HPP
#define BOOST_GEOM_WRAPPER_HPP

#include "boost_geometries.hpp"

namespace gpkg {
    double area(GeometryPtr& geometry);

    int is_valid(GeometryPtr& geometry);
    int is_simple(GeometryPtr& geometry);
}

#endif