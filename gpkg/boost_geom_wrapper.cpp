#include <boost/geometry/geometry.hpp>
#include "boost_geom_wrapper.hpp"

namespace gpkg {
    namespace detail {
        template<typename Visitor>
        struct geometry_ptr_visitor : boost::static_visitor<typename Visitor::result_type> {
          typename Visitor::result_type null_result;

          geometry_ptr_visitor(typename Visitor::result_type null_result) : null_result(null_result) {
          }

          typename Visitor::result_type operator()(const boost::blank &value) const {
            return null_result;
          }

          template<typename Geometry>
          typename Visitor::result_type operator()(const Geometry *value) const {
            return value != nullptr ? Visitor()(*value) : null_result;
          }
        };

        struct is_valid : boost::static_visitor<int> {
          template<typename Geometry>
          int operator()(const Geometry &geom) const {
            return boost::geometry::is_valid(geom);
          }

          int operator()(const gpkg::GeometryCollection &geom) const {
            for (auto it = geom.cbegin(); it < geom.cend(); ++it) {
              if (!boost::apply_visitor(*this, *it)) {
                return false;
              }
            }
            return true;
          }
        };

        struct is_simple : boost::static_visitor<int> {
          template<typename Geometry>
          int operator()(const Geometry &geom) const {
            return boost::geometry::is_simple(geom);
          }

          int operator()(const gpkg::GeometryCollection &geom) const {
            for (auto it = geom.cbegin(); it < geom.cend(); ++it) {
              if (!boost::apply_visitor(*this, *it)) {
                return false;
              }
            }
            return true;
          }
        };

        struct area : boost::static_visitor<double> {
          template<typename Geometry>
          double operator()(const Geometry &geom) const {
            return boost::geometry::area(geom);
          }

          double operator()(const gpkg::GeometryCollection &geom) const {
            double total = 0.0;

            for (auto it = geom.cbegin(); it < geom.cend(); ++it) {
              total += boost::apply_visitor(*this, *it);
            }

            return total;
          }
        };
    }

    int is_valid(GeometryPtr& geometry) {
      return boost::apply_visitor(detail::geometry_ptr_visitor<detail::is_valid>(true), geometry);
    }

    int is_simple(GeometryPtr& geometry) {
      return boost::apply_visitor(detail::geometry_ptr_visitor<detail::is_simple>(true), geometry);
    }

    double area(GeometryPtr& geometry) {
      return boost::apply_visitor(detail::geometry_ptr_visitor<detail::area>(0.0), geometry);
    }
}